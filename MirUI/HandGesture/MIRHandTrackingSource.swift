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

    /// These flags and the generation token are accessed only on `sessionQueue`.
    private var configured = false
    private var running = false
    private var sessionGeneration: UInt64 = 0

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

            // Reserve this start operation on the session queue before asking
            // for permission. `stop()` can then invalidate exactly this
            // generation even while the system permission dialog is active.
            let generation: UInt64 = sessionQueue.sync {
                sessionGeneration &+= 1
                return sessionGeneration
            }

            // Permission is asynchronous and must not block the camera queue.
            Task.detached { [weak self] in
                guard let self else { return }

                let granted = await AVCaptureDevice.requestAccess(for: .video)
                guard granted, !Task.isCancelled else {
                    self.finishStream()
                    return
                }

                // AVFoundation session configuration and startRunning are both
                // serialized on exactly the same queue. The generation check
                // prevents a permission callback from resurrecting a stopped
                // session.
                self.sessionQueue.async { [weak self] in
                    guard let self else { return }
                    guard generation == self.sessionGeneration else { return }
                    self.configureIfNeededOnSessionQueue()
                    guard self.configured, !self.running else { return }
                    guard generation == self.sessionGeneration else { return }
                    self.session.startRunning()
                    self.running = self.session.isRunning
                }
            }
        }
    }

    func stop() {
        // Finish the stream immediately so consumers stop processing frames.
        finishStream()

        // Invalidate the pending start before stopping the session. Any
        // permission callback queued later will carry the old generation and
        // will be ignored on sessionQueue.
        sessionQueue.async { [weak self] in
            guard let self else { return }
            self.sessionGeneration &+= 1
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

    /// Must be called only from `sessionQueue`.
    ///
    /// Do not use `dispatchPrecondition(.onQueue:)` here. The queue ownership is
    /// already enforced by the single `sessionQueue.async` call site, while a
    /// debug-only queue assertion can itself terminate the application with
    /// `_dispatch_assert_queue_fail` if AVFoundation changes execution context.
    private func configureIfNeededOnSessionQueue() {
        guard !configured else { return }

        session.beginConfiguration()
        defer { session.commitConfiguration() }

        session.sessionPreset = .high

        guard let device = AVCaptureDevice.default(for: .video),
              let input = try? AVCaptureDeviceInput(device: device),
              session.canAddInput(input) else {
            return
        }

        session.addInput(input)
        output.alwaysDiscardsLateVideoFrames = true
        output.setSampleBufferDelegate(self, queue: visionQueue)

        guard session.canAddOutput(output) else {
            return
        }

        session.addOutput(output)

        if let connection = output.connection(with: .video),
           connection.isVideoRotationAngleSupported(90) {
            connection.videoRotationAngle = 90
        }

        configured = true
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
