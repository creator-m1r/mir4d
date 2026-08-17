import Foundation
import AVFoundation
import Vision
import CoreGraphics
import Combine

struct MIRHandPoint: Equatable, Sendable {
    let x: CGFloat
    let y: CGFloat
    let confidence: Float
}

struct MIRHandPose: Equatable, Sendable {
    let indexTip: MIRHandPoint
    let thumbTip: MIRHandPoint
    let middleTip: MIRHandPoint
    let wrist: MIRHandPoint
    let pinchDistance: CGFloat
    let openness: CGFloat
    let timestamp: Date
}

@MainActor
final class MIRHandTrackingSession: NSObject, ObservableObject {
    @Published private(set) var pose: MIRHandPose?
    @Published private(set) var isRunning = false
    @Published private(set) var cameraAvailable = false

    private let session = AVCaptureSession()
    private let output = AVCaptureVideoDataOutput()
    private let queue = DispatchQueue(label: "com.mir4d.hand-tracking", qos: .userInteractive)
    private let visionQueue = DispatchQueue(label: "com.mir4d.hand-vision", qos: .userInteractive)
    private var configured = false

    func start() {
        guard !isRunning else { return }
        Task { @MainActor in
            guard await AVCaptureDevice.requestAccess(for: .video) else { return }
            configureIfNeeded()
            guard !session.isRunning else { isRunning = true; return }
            queue.async { [weak self] in self?.session.startRunning() }
            isRunning = true
        }
    }

    func stop() {
        queue.async { [weak self] in self?.session.stopRunning() }
        isRunning = false
        pose = nil
    }

    private func configureIfNeeded() {
        guard !configured else { return }
        session.beginConfiguration()
        session.sessionPreset = .high
        guard let device = AVCaptureDevice.default(for: .video), let input = try? AVCaptureDeviceInput(device: device), session.canAddInput(input) else {
            session.commitConfiguration(); return
        }
        session.addInput(input)
        output.alwaysDiscardsLateVideoFrames = true
        output.setSampleBufferDelegate(self, queue: visionQueue)
        guard session.canAddOutput(output) else { session.commitConfiguration(); return }
        session.addOutput(output)
        if let connection = output.connection(with: .video) { connection.videoOrientation = .portrait }
        session.commitConfiguration()
        configured = true
        cameraAvailable = true
    }

    nonisolated private func process(_ buffer: CVPixelBuffer, owner: MIRHandTrackingSession) {
        let request = VNDetectHumanHandPoseRequest { request, _ in
            guard let observation = request.results?.first as? VNHumanHandPoseObservation,
                  let wrist = try? observation.recognizedPoint(.wrist),
                  let index = try? observation.recognizedPoint(.indexTip),
                  let thumb = try? observation.recognizedPoint(.thumbTip),
                  let middle = try? observation.recognizedPoint(.middleTip),
                  wrist.confidence > 0.45, index.confidence > 0.45 else { return }
            let pose = MIRHandPose(
                indexTip: .init(x: index.location.x, y: index.location.y, confidence: index.confidence),
                thumbTip: .init(x: thumb.location.x, y: thumb.location.y, confidence: thumb.confidence),
                middleTip: .init(x: middle.location.x, y: middle.location.y, confidence: middle.confidence),
                wrist: .init(x: wrist.location.x, y: wrist.location.y, confidence: wrist.confidence),
                pinchDistance: hypot(index.location.x - thumb.location.x, index.location.y - thumb.location.y),
                openness: hypot(middle.location.x - wrist.location.x, middle.location.y - wrist.location.y),
                timestamp: Date()
            )
            Task { @MainActor in owner.pose = pose }
        }
        request.maximumHandCount = 2
        try? VNImageRequestHandler(cvPixelBuffer: buffer, orientation: .leftMirrored).perform([request])
    }
}

extension MIRHandTrackingSession: AVCaptureVideoDataOutputSampleBufferDelegate {
    nonisolated func captureOutput(_ output: AVCaptureOutput, didOutput sampleBuffer: CMSampleBuffer, from connection: AVCaptureConnection) {
        guard let buffer = CMSampleBufferGetImageBuffer(sampleBuffer) else { return }
        process(buffer, owner: self)
    }
}
