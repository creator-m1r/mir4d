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
            Task {
                if frames.isEmpty { continuation.finish(); return }
                var index = 0
                while true {
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

final class MIRCameraTrackingSource: NSObject, MIRHandTrackingSource, @unchecked Sendable {
    private let session = AVCaptureSession()
    private let output = AVCaptureVideoDataOutput()
    private let sessionQueue = DispatchQueue(label: "com.mir4d.hand-camera", qos: .userInteractive)
    private let visionQueue = DispatchQueue(label: "com.mir4d.hand-vision", qos: .userInteractive)
    private let continuationLock = OSAllocatedUnfairLock<AsyncStream<[MIRHandPose]>.Continuation?>(initialState: nil)
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

            Task {
                let granted = await AVCaptureDevice.requestAccess(for: .video)
                guard granted else {
                    continuationLock.withLock { continuation in
                        continuation?.finish()
                    }
                    continuationLock.withLock { continuation in
                        continuation = nil
                    }
                    return
                }

                self.configureIfNeeded()
                self.sessionQueue.async { [weak self] in
                    self?.session.startRunning()
                    self?.running = true
                }
            }
        }
    }

    func stop() {
        continuationLock.withLock { continuation in
            continuation?.finish()
        }

        continuationLock.withLock { continuation in
            continuation = nil
        }

        sessionQueue.async { [weak self] in
            self?.session.stopRunning()
            self?.running = false
        }
    }

    private func configureIfNeeded() {
        guard !configured else { return }

        session.beginConfiguration()
        session.sessionPreset = .high

        guard let device = AVCaptureDevice.default(for: .video),
              let input = try? AVCaptureDeviceInput(device: device),
              session.canAddInput(input) else {
            session.commitConfiguration()
            return
        }

        session.addInput(input)
        output.alwaysDiscardsLateVideoFrames = true
        output.setSampleBufferDelegate(self, queue: visionQueue)

        guard session.canAddOutput(output) else {
            session.commitConfiguration()
            return
        }

        session.addOutput(output)

        if let connection = output.connection(with: .video),
           connection.isVideoRotationAngleSupported(90) {
            connection.videoRotationAngle = 90
        }

        session.commitConfiguration()
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

        return MIRHandPose(
            landmarks: landmarks,
            confidence: averageConfidence,
            timestamp: CACurrentMediaTime()
        )
    }
}
