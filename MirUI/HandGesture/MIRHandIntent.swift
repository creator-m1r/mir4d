import Foundation
import simd

/// Lifecycle phase of an emitted hand intent.
public enum MIRHandIntentPhase: String, Sendable {
    case began
    case changed
    case ended
    case cancelled
}

/// Structured spatial intention produced by the hand module.
///
/// Recognition never triggers an operation directly. The intent is the only
/// contract between the hand module and the rest of MIR 4D (Spatial Menu, CAD).
public struct MIRHandIntent: Sendable {
    public let gesture: MIRHandGesture
    public let phase: MIRHandIntentPhase

    public let position: SIMD3<Double>
    public let direction: SIMD3<Double>

    public let strength: Double
    public let confidence: Double

    public let timestamp: Date

    public init(
        gesture: MIRHandGesture,
        phase: MIRHandIntentPhase,
        position: SIMD3<Double>,
        direction: SIMD3<Double>,
        strength: Double,
        confidence: Double,
        timestamp: Date = Date()
    ) {
        self.gesture = gesture
        self.phase = phase
        self.position = position
        self.direction = direction
        self.strength = strength
        self.confidence = confidence
        self.timestamp = timestamp
    }

    /// Bridge into the universal, device-independent intent bus.
    func toMIRIntent() -> MIRIntent {
        let phase: MIRIntent.Phase
        switch self.phase {
        case .began: phase = .selection
        case .changed: phase = .preview
        case .ended: phase = .confirmation
        case .cancelled: phase = .cancel
        }
        return MIRIntent(
            source: .spatial,
            phase: phase,
            action: gesture.type.rawValue,
            directionRadians: self.direction.z != 0 ? atan2(self.direction.y, self.direction.x) : nil,
            value: self.strength,
            confidence: self.confidence,
            timestamp: self.timestamp
        )
    }
}

/// Publishes hand intents onto the shared `MIRIntentRouter`.
/// Calling `publish` must happen on the main actor (the router is main-isolated).
@MainActor
struct MIRHandIntentEmitter {
    func publish(_ intent: MIRHandIntent) {
        MIRIntentRouter.shared.publish(intent.toMIRIntent())
    }
}
