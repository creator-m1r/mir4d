import Foundation
import simd

/// Interprets two tracked hands as spatial manipulation gestures.
/// Pure, side-effect-free and actor-agnostic so it can run in the background
/// recognition pipeline. It emits a neutral result; the session turns it into a
/// `MIRHandIntent`. Scene geometry is never mutated here.
struct MIRAirGestureController: Sendable {
    enum Gesture: Sendable {
        case none
        case translate
        case scale
        case rotate
        case twoHandPinch
        case twoHandGrab
    }

    struct Configuration: Sendable {
        var minimumHandDistance: Double = 0.08
        var scaleDeadZone: Double = 0.015
        var rotationDeadZone: Double = 0.025
        var translationDeadZone: Double = 0.008
    }

    var configuration = Configuration()

    private var previousCenter: SIMD3<Double>?
    private var previousDistance: Double?
    private var previousAngle: Double?

    /// Evaluate a two-hand frame. `leftPinch`/`rightPinch` are the per-hand
    /// pinch strengths (0...1); `leftGrab`/`rightGrab` indicate closed grasps.
    /// Returns the active gesture together with a strength and confidence, or
    /// `nil` when the hands are not yet in a recognised spatial relationship.
    mutating func ingest(
        left: SIMD3<Double>,
        right: SIMD3<Double>,
        leftPinch: Double,
        rightPinch: Double,
        leftGrab: Bool,
        rightGrab: Bool,
        timestamp: Date
    ) -> (type: MIRHandGestureType, strength: Double, confidence: Double, value: Double)? {
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

        if leftPinch > 0.7 && rightPinch > 0.7 {
            return (.twoHandPinch, (leftPinch + rightPinch) * 0.5, 0.92, distance)
        }
        if leftGrab && rightGrab {
            return (.twoHandGrab, 1.0, 0.9, distance)
        }
        if abs(scaleDelta) > configuration.scaleDeadZone {
            return (.twoHandScale, min(abs(scaleDelta) * 10, 1), 0.9, scaleDelta)
        }
        if abs(rotationDelta) > configuration.rotationDeadZone {
            return (.twoHandRotate, min(abs(rotationDelta) * 10, 1), 0.9, rotationDelta)
        }
        if translation > configuration.translationDeadZone {
            return (.twoHandTranslate, min(translation * 10, 1), 0.88, translation)
        }
        return nil
    }

    mutating func reset() {
        previousCenter = nil
        previousDistance = nil
        previousAngle = nil
    }

    private func normalizedAngle(_ value: Double) -> Double {
        var angle = value
        while angle > .pi { angle -= 2 * .pi }
        while angle < -.pi { angle += 2 * .pi }
        return angle
    }
}
