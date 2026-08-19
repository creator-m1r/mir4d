import Foundation
import CoreGraphics
import simd
import AVFoundation
import Vision
import os

/// Whether a tracking source can currently provide frames.
enum MIRHandTrackingAvailability: Sendable {
    case notDetermined
    case available
    case unavailable
}

/// A replaceable source of hand poses.
protocol MIRHandTrackingSource: Sendable {
    var availability: MIRHandTrackingAvailability { get }
    func start() -> AsyncStream<[MIRHandPose]>
    func stop()
}

// MARK: - Mock source

struct MIRMockTrackingSource: MIRHandTrackingSource {
    enum Mode: Sendable { case once, loop }

    let frames: [[MIRHandPose]]
    var mode: Mode = .once
    var availability: MIRHandTrackingAvailability = .available

    func start() -> AsyncStream<[MIRHandPose]> {
        AsyncStream { continuation in
            let frames = self.frames
            let mode = self.mode
            Task.detached {
                if frames.isEmpty {
                    continuation.finish()
                    return
                }

                var index = 0
                while !Task.isCancelled {
                    continuation.yield(frames[index])
                    index += 1
                    if index >= frames.count {
                        if mode == .loop {
                            index = 0
                        } else {
                            break
                        }
                    }
                }
                continuation.finish()
            }
        }
    }

    func stop() {}
}

// MARK: - Camera source

/// Camera/Vision source.
///
/// Important concurrency rule: every mutation of AVCaptureSession, its inputs
/// and outputs is serialized on `sessionQueue`. Vision callbacks only read the
/// sample buffer and publish immutable pose snapshots through the locked
/// AsyncStream continuation. This prevents AVFoundation/libdispatch queue
/// assertions when the hand module is started/stopped from SwiftUI or a
/// background task.
final class MIRCameraTrackingSource: NSObject, MIRHandTrackingSource, @unchecked Sendable {
    private let session = AVCaptureSession()
    private let output = AVCaptureVideoDataOutput()
    private let sessionQueue = DispatchQueue(label: "com.mir4d.hand-camera", qos: .userInteractive)
    private let visionQueue = DispatchQueue(label: "com.mir4d.hand-vision", qos: .userInteractive)
    private let continuationLock = OSAllocatedUnfairLock<AsyncStream<[MIRHandPose]>.Continuation?>(initialState: nil)

    /// Защищает generation-счётчик (ТЗ §11). Каждый start()/stop() увеличивают
    /// поколение; отложенный permission-callback запускает камеру только если
    /// его поколение совпадает с текущим — иначе startRunning() после stop()
    /// не происходит.
    private let generationLock = OSAllocatedUnfairLock<UInt64>(initialState: 0)

    /// These flags are accessed only on `sessionQueue`.
    private var configured = false
    private var running = false

    var availability: MIRHandTrackingAvailability {
        let status = AVCaptureDevice.authorizationStatus(for: .video)
        guard status != .denied, status != .restricted else { return .unavailable }
        guard AVCaptureDevice.default(for: .video) != nil else { return .unavailable }
        return .available
    }

    func start() -> AsyncStream<[MIRHandPose]> {
        AsyncStream { [weak self] continuation in
            guard let self else {
                continuation.finish()
                return
            }

            continuationLock.withLock { $0 = continuation }

            // Захватываем поколение для ЭТОГО start(). Любой последующий stop()
            // (или повторный start()) увеличит поколение, и отложенный запуск
            // камеры ниже будет пропущен (ТЗ §11, Test 5).
            let myGeneration = generationLock.withLock { state in
                state &+= 1
                return state
            }

            // Permission is asynchronous and must not block the camera queue.
            Task.detached { [weak self] in
                guard let self else { return }

                // TCC-запрос разрешения должен выполняться на главном потоке:
                // иначе remote view сервис диалога разрешений (RemoteViewService)
                // завершается с ViewBridge error (ТЗ §5, macOS TCC quirk).
                let granted: Bool = await withCheckedContinuation { continuation in
                    Task { @MainActor in
                        continuation.resume(returning: await AVCaptureDevice.requestAccess(for: .video))
                    }
                }
                guard granted, !Task.isCancelled else {
                    self.finishStream()
                    return
                }

                // AVFoundation session configuration and startRunning are both
                // serialized on exactly the same queue.
                self.sessionQueue.async { [weak self] in
                    guard let self else { return }

                    // Поколение могло измениться (stop() пришёл до ответа
                    // permission) — в этом случае НЕ запускаем камеру.
                    let current = self.generationLock.withLock { $0 }
                    guard current == myGeneration else {
                        self.finishStream()
                        return
                    }

                    self.configureIfNeededOnSessionQueue {
                        guard !self.running else { return }
                        self.session.startRunning()
                        self.running = self.session.isRunning
                    }
                }
            }
        }
    }

    func stop() {
        // Finish the stream immediately so consumers stop processing frames.
        finishStream()

        // Инвалидируем любой отложенный start от предыдущего поколения.
        generationLock.withLock { $0 &+= 1 }

        // All AVCaptureSession mutations remain serialized on sessionQueue.
        sessionQueue.async { [weak self] in
            guard let self else { return }
            if self.session.isRunning {
                self.session.stopRunning()
            }
            self.running = false
        }
    }

    private func finishStream() {
        continuationLock.withLock { continuation in
            continuation?.finish()
            continuation = nil
        }
    }

    /// Конфигурирует `AVCaptureSession` строго на `sessionQueue`. Тело всегда
    /// выполняется через `sessionQueue.async`, поэтому вызов из любой другой
    /// очереди (visionQueue / Thread 12 при старте сессии из 3D-сцены, либо из
    /// `captureOutput`) безопасен и не вызывает EXC_BREAKPOINT в
    /// `dispatch_assert_queue_fail` (ТЗ §5). Внутри вызываются
    /// `AVCaptureSession.beginConfiguration` / `addInput`, которые внутренне
    /// assert-ят выполнение именно на очереди сессии. `then` вызывается после
    /// успешной конфигурации (также на `sessionQueue`).
    private func configureIfNeededOnSessionQueue(then startIfConfigured: @Sendable @escaping () -> Void) {
        sessionQueue.async { [weak self] in
            guard let self else { return }
            guard !self.configured else {
                startIfConfigured()
                return
            }

            self.session.beginConfiguration()
            self.session.sessionPreset = .high

            guard let device = AVCaptureDevice.default(for: .video),
                  let input = try? AVCaptureDeviceInput(device: device),
                  self.session.canAddInput(input) else {
                self.session.commitConfiguration()
                return
            }

            self.session.addInput(input)
            self.output.alwaysDiscardsLateVideoFrames = true
            self.output.setSampleBufferDelegate(self, queue: self.visionQueue)

            guard self.session.canAddOutput(self.output) else {
                self.session.commitConfiguration()
                return
            }

            self.session.addOutput(self.output)

            if let connection = self.output.connection(with: .video),
               connection.isVideoRotationAngleSupported(90) {
                connection.videoRotationAngle = 90
            }

            // Коммитим конфигурацию ДО startRunning (иначе сессия может
            // заблокироваться внутри открытой транзакции конфигурации).
            self.session.commitConfiguration()
            self.configured = true
            startIfConfigured()
        }
    }
}

extension MIRCameraTrackingSource: AVCaptureVideoDataOutputSampleBufferDelegate {
    nonisolated func captureOutput(
        _ output: AVCaptureOutput,
        didOutput sampleBuffer: CMSampleBuffer,
        from connection: AVCaptureConnection
    ) {
        guard let buffer = CMSampleBufferGetImageBuffer(sampleBuffer) else { return }

        let request = VNDetectHumanHandPoseRequest()
        request.maximumHandCount = 2

        do {
            try VNImageRequestHandler(
                cvPixelBuffer: buffer,
                orientation: .up,
                options: [:]
            ).perform([request])
        } catch {
            return
        }

        guard let observations = request.results else { return }
        let poses: [MIRHandPose] = observations.compactMap { pose(from: $0) }

        continuationLock.withLock { continuation in
            continuation?.yield(poses)
            return
        }
    }

    private func pose(from observation: VNHumanHandPoseObservation) -> MIRHandPose? {
        let jointMap: [(LandmarkID, VNHumanHandPoseObservation.JointName)] = [
            (.wrist, .wrist),
            (.thumbCMC, .thumbCMC),
            (.thumbMCP, .thumbMP),
            (.thumbIP, .thumbIP),
            (.thumbTip, .thumbTip),
            (.indexMCP, .indexMCP),
            (.indexPIP, .indexPIP),
            (.indexDIP, .indexDIP),
            (.indexTip, .indexTip),
            (.middleMCP, .middleMCP),
            (.middlePIP, .middlePIP),
            (.middleDIP, .middleDIP),
            (.middleTip, .middleTip),
            (.ringMCP, .ringMCP),
            (.ringPIP, .ringPIP),
            (.ringDIP, .ringDIP),
            (.ringTip, .ringTip),
            (.littleMCP, .littleMCP),
            (.littlePIP, .littlePIP),
            (.littleDIP, .littleDIP),
            (.littleTip, .littleTip)
        ]

        var landmarks: [MIRHandLandmark] = []
        var confidences: [Double] = []

        for (id, name) in jointMap {
            guard let point = try? observation.recognizedPoint(name),
                  point.confidence > 0.2 else {
                return nil
            }

            landmarks.append(
                MIRHandLandmark(
                    id: id,
                    normalizedPosition: SIMD3(
                        Double(point.location.x),
                        Double(point.location.y),
                        0
                    ),
                    confidence: Double(point.confidence)
                )
            )

            confidences.append(Double(point.confidence))
        }

        let averageConfidence = confidences.isEmpty
            ? 0
            : confidences.reduce(0, +) / Double(confidences.count)

        let handedness: Handedness
        switch observation.chirality {
        case .left:
            handedness = .left
        case .right:
            handedness = .right
        case .unknown:
            handedness = .unknown
        @unknown default:
            handedness = .unknown
        }

        return MIRHandPose(
            id: UUID(),
            handedness: handedness,
            landmarks: landmarks,
            palmPosition: palmPosition(from: landmarks),
            palmNormal: palmNormal(from: landmarks),
            confidence: averageConfidence,
            timestamp: Date()
        )
    }

    private func palmPosition(from landmarks: [MIRHandLandmark]) -> SIMD3<Double> {
        let ids: [LandmarkID] = [.wrist, .indexMCP, .middleMCP, .ringMCP, .littleMCP]
        let points = ids.compactMap { id in
            landmarks.first(where: { $0.id == id })?.normalizedPosition
        }

        guard !points.isEmpty else { return .zero }
        return points.reduce(.zero, +) / Double(points.count)
    }

    private func palmNormal(from landmarks: [MIRHandLandmark]) -> SIMD3<Double> {
        guard
            let wrist = landmarks.first(where: { $0.id == .wrist })?.normalizedPosition,
            let index = landmarks.first(where: { $0.id == .indexMCP })?.normalizedPosition,
            let little = landmarks.first(where: { $0.id == .littleMCP })?.normalizedPosition
        else {
            return SIMD3<Double>(0, 0, 1)
        }

        let a = index - wrist
        let b = little - wrist
        let normal = simd_cross(a, b)
        let length = simd_length(normal)

        guard length > 0.000001 else {
            return SIMD3<Double>(0, 0, 1)
        }

        return normal / length
    }
}
