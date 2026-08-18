import Foundation
import CoreGraphics
import simd
import AVFoundation
import Vision

/// Whether a tracking source can currently provide frames.
enum MIRHandTrackingAvailability: Sendable {
    case notDetermined
    case available
    case unavailable
}

/// A replaceable source of hand poses.
///
/// The module is decoupled from any specific camera technology. Built-in
/// implementations: `MIRCameraTrackingSource` (MacBook / iPad camera) and
/// `MIRMockTrackingSource` (synthetic data for tests and the Spatial Menu).
/// External USB cameras, depth cameras, ARKit and spatial devices can be added
/// by implementing this protocol.
protocol MIRHandTrackingSource: Sendable {
    var availability: MIRHandTrackingAvailability { get }
    /// Begin producing pose frames. The returned stream ends when `stop()` is called.
    func start() -> AsyncStream<[MIRHandPose]>
    func stop()
}

// MARK: - Mock source

/// Feeds pre-recorded synthetic poses. Enables testing the full pipeline and
/// the Spatial Menu without any camera hardware.
struct MIRMockTrackingSource: MIRHandTrackingSource {
    enum Mode: Sendable { case once, loop }

    let frames: [[MIRHandPose]]
    var mode: Mode = .once
    var availability: MIRHandTrackingAvailability = .available

    func start() -> AsyncStream<[MIRHandPose]> {
        AsyncStream { continuation in
            let frames = self.frames
            if frames.isEmpty { continuation.finish(); return }
            func emit(_ index: Int) {
                guard index < frames.count else {
                    if self.mode == .loop { emit(0) } else { continuation.finish() }
                    return
                }
                continuation.yield(frames[index])
                emit(index + 1)
            }
            emit(0)
        }
    }

    func stop() {}
}

// MARK: - Camera source

/// Default source: built-in camera via AVFoundation + Vision hand-pose detection.
/// All heavy work runs off the main actor; failures degrade to `unavailable`
/// instead of crashing the app.
final class MIRCameraTrackingSource: NSObject, MIRHandTrackingSource, @unchecked Sendable {
    private let session = AVCaptureSession()
    private let output = AVCaptureVideoDataOutput()
    private let sessionQueue = DispatchQueue(label: "com.mir4d.hand-camera", qos: .userInteractive)
    private let visionQueue = DispatchQueue(label: "com.mir4d.hand-vision", qos: .userInteractive)
    private var continuation: AsyncStream<[MIRHandPose]>.Continuation?
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
            guard let self else { continuation.finish(); return }
            self.continuation = continuation
            Task {
                let granted = await AVCaptureDevice.requestAccess(for: .video)
                guard granted else {
                    self.continuation?.finish()
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
        sessionQueue.async { [weak self] in
            self?.session.stopRunning()
            self?.running = false
        }
        continuation?.finish()
        continuation = nil
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
        guard session.canAddOutput(output) else { session.commitConfiguration(); return }
        session.addOutput(output)
        if let connection = output.connection(with: .video) {
            connection.videoOrientation = .portrait
        }
        session.commitConfiguration()
        configured = true
    }
}

extension MIRCameraTrackingSource: AVCaptureVideoDataOutputSampleBufferDelegate {
    nonisolated func captureOutput(_ output: AVCaptureOutput, didOutput sampleBuffer: CMSampleBuffer, from connection: AVCaptureConnection) {
        guard let buffer = CMSampleBufferGetImageBuffer(sampleBuffer) else { return }
        let request = VNDetectHumanHandPoseRequest()
        request.maximumHandCount = 2
        do {
            try VNImageRequestHandler(cvPixelBuffer: buffer, orientation: .up, options: [:]).perform([request])
        } catch {
            return
        }
        guard let observations = request.results as? [VNHumanHandPoseObservation] else { return }
        let poses: [MIRHandPose] = observations.compactMap { pose(from: $0) }
        continuation?.yield(poses)
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
                  point.confidence > 0.2 else { return nil }
            landmarks.append(MIRHandLandmark(
                id: id,
                normalizedPosition: SIMD3(Double(point.location.x), Double(point.location.y), 0),
                confidence: Double(point.confidence)
            ))
            confidences.append(Double(point.confidence))
        }

        let palm = computePalm(landmarks: landmarks)
        let handedness: Handedness
        switch observation.chirality {
        case .left: handedness = .left
        case .right: handedness = .right
        default: handedness = .unknown
        }

        return MIRHandPose(
            id: UUID(),
            handedness: handedness,
            landmarks: landmarks,
            palmPosition: palm.position,
            palmNormal: palm.normal,
            confidence: confidences.min() ?? 0,
            timestamp: Date()
        )
    }

    private func computePalm(landmarks: [MIRHandLandmark]) -> (position: SIMD3<Double>, normal: SIMD3<Double>) {
        let ids: [LandmarkID] = [.wrist, .indexMCP, .middleMCP, .ringMCP, .littleMCP]
        let pts = ids.compactMap { id in landmarks.first(where: { $0.id == id })?.normalizedPosition }
        guard !pts.isEmpty else { return (.zero, .zero) }
        let position = pts.reduce(SIMD3<Double>(0, 0, 0)) { $0 + $1 } / Double(pts.count)

        let v1: SIMD3<Double>
        let v2: SIMD3<Double>
        if let wrist = landmarks.first(where: { $0.id == .wrist })?.normalizedPosition,
           let index = landmarks.first(where: { $0.id == .indexMCP })?.normalizedPosition,
           let little = landmarks.first(where: { $0.id == .littleMCP })?.normalizedPosition {
            v1 = index - wrist
            v2 = little - wrist
        } else {
            return (position, SIMD3(0, 0, 1))
        }
        let z = v1.x * v2.y - v1.y * v2.x
        let normal = z >= 0 ? SIMD3<Double>(0, 0, 1) : SIMD3<Double>(0, 0, -1)
        return (position, normal)
    }
}
