import Foundation

/// Interprets two tracked hands as spatial manipulation gestures.
/// It emits intent data only; scene mutation remains in the CAD/geometry layers.
@MainActor
final class MIRAirGestureController {
    enum Gesture: Equatable {
        case none
        case translate
        case scale
        case rotate
        case twoHandSculpt
    }

    struct Configuration: Equatable, Sendable {
        var minimumHandDistance: Double = 0.08
        var scaleDeadZone: Double = 0.015
        var rotationDeadZone: Double = 0.025
        var translationDeadZone: Double = 0.008
    }

    private(set) var gesture: Gesture = .none
    private(set) var configuration = Configuration()
    private var previousCenter: SIMD3<Double>?
    private var previousDistance: Double?
    private var previousAngle: Double?

    func update(configuration: Configuration) {
        self.configuration = configuration
    }

    func reset() {
        gesture = .none
        previousCenter = nil
        previousDistance = nil
        previousAngle = nil
    }

    func ingest(left: SIMD3<Double>, right: SIMD3<Double>, timestamp: Date = Date()) -> MIRIntent? {
        let delta = right - left
        let distance = simd_length(delta)
        guard distance >= configuration.minimumHandDistance else {
            reset()
            return nil
        }

        let center = (left + right) * 0.5
        let angle = atan2(delta.y, delta.x)
        let translation = previousCenter.map { simd_length(center - $0) } ?? 0
        let scaleDelta = previousDistance.map { distance - $0 } ?? 0
        let rotationDelta = previousAngle.map { normalizedAngle(angle - $0) } ?? 0

        previousCenter = center
        previousDistance = distance
        previousAngle = angle

        if abs(scaleDelta) > configuration.scaleDeadZone {
            gesture = .scale
            return MIRIntent(source: .spatial, phase: .selection, action: "scale", value: scaleDelta, confidence: 0.92, timestamp: timestamp)
        }

        if abs(rotationDelta) > configuration.rotationDeadZone {
            gesture = .rotate
            return MIRIntent(source: .spatial, phase: .selection, action: "rotate", directionRadians: rotationDelta, confidence: 0.9, timestamp: timestamp)
        }

        if translation > configuration.translationDeadZone {
            gesture = .translate
            return MIRIntent(source: .spatial, phase: .selection, action: "translate", directionRadians: atan2((center - (previousCenter ?? center)).y, (center - (previousCenter ?? center)).x), value: translation, confidence: 0.88, timestamp: timestamp)
        }

        gesture = .twoHandSculpt
        return MIRIntent(source: .spatial, phase: .preview, action: "twoHandSculpt", value: distance, confidence: 0.84, timestamp: timestamp)
    }

    private func normalizedAngle(_ value: Double) -> Double {
        var angle = value
        while angle > .pi { angle -= 2 * .pi }
        while angle < -.pi { angle += 2 * .pi }
        return angle
    }
}
