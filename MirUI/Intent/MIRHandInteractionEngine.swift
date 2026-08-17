import Foundation
import CoreGraphics

/// Converts tracked hand poses into device-independent spatial intents.
/// Geometry mutation is deliberately outside this layer.
@MainActor
final class MIRHandInteractionEngine {
    enum Mode: Equatable {
        case idle
        case draw
        case sculpt
        case grab
        case navigate
    }

    struct Configuration: Equatable, Sendable {
        var pinchThreshold: CGFloat = 0.055
        var grabThreshold: CGFloat = 0.11
        var releaseThreshold: CGFloat = 0.075
        var minimumMotion: CGFloat = 0.006
        var smoothing: CGFloat = 0.22
    }

    private(set) var mode: Mode = .idle
    private(set) var configuration = Configuration()
    private var smoothedPoint: CGPoint?
    private var previousPoint: CGPoint?
    private var lastTimestamp: Date?

    func update(configuration: Configuration) {
        self.configuration = configuration
    }

    func reset() {
        mode = .idle
        smoothedPoint = nil
        previousPoint = nil
        lastTimestamp = nil
    }

    func ingest(_ pose: MIRHandPose) -> MIRIntent? {
        let raw = CGPoint(x: pose.indexTip.x, y: pose.indexTip.y)
        let point = smooth(raw)
        let velocity = motion(from: previousPoint, to: point, timestamp: pose.timestamp)
        previousPoint = point
        lastTimestamp = pose.timestamp

        if pose.pinchDistance <= configuration.pinchThreshold {
            mode = .sculpt
            return MIRIntent(source: .spatial, phase: .selection, action: "sculpt", directionRadians: atan2(velocity.dy, velocity.dx), value: Double(pose.pinchDistance), confidence: Double(pose.indexTip.confidence))
        }

        if pose.openness <= configuration.grabThreshold {
            mode = .grab
            return MIRIntent(source: .spatial, phase: .selection, action: "grab", directionRadians: atan2(velocity.dy, velocity.dx), value: Double(velocity.magnitude), confidence: Double(pose.wrist.confidence))
        }

        if velocity.magnitude >= configuration.minimumMotion {
            mode = .draw
            return MIRIntent(source: .spatial, phase: .preview, action: "draw", directionRadians: atan2(velocity.dy, velocity.dx), value: Double(velocity.magnitude), confidence: Double(pose.indexTip.confidence))
        }

        mode = .idle
        return MIRIntent(source: .spatial, phase: .attention, confidence: Double(pose.wrist.confidence))
    }

    private func smooth(_ raw: CGPoint) -> CGPoint {
        guard let previous = smoothedPoint else { smoothedPoint = raw; return raw }
        let a = configuration.smoothing
        let result = CGPoint(x: previous.x + (raw.x - previous.x) * a, y: previous.y + (raw.y - previous.y) * a)
        smoothedPoint = result
        return result
    }

    private func motion(from previous: CGPoint?, to current: CGPoint, timestamp: Date) -> (dx: CGFloat, dy: CGFloat, magnitude: CGFloat) {
        guard let previous, let lastTimestamp else { return (0, 0, 0) }
        let dt = max(timestamp.timeIntervalSince(lastTimestamp), 1.0 / 120.0)
        let dx = (current.x - previous.x) / CGFloat(dt)
        let dy = (current.y - previous.y) / CGFloat(dt)
        return (dx, dy, hypot(dx, dy))
    }
}
