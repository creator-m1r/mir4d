import Foundation
import simd

/// Contact state of a hand against the (virtual or real) interaction volume.
/// It is the module's bridge toward downstream consumers such as
/// `MIRAirContactField`, `MIRAirSculptController` and `MIRSurfaceContactSolver`.
/// The hand module never creates geometry itself.
struct MIRHandContact: Sendable {
    enum State: String, Sendable {
        case hovering
        case touching
        case pressing
        case released
    }

    let position: SIMD3<Double>
    let velocity: SIMD3<Double>
    let radius: Double
    let strength: Double
    let state: State
    let timestamp: Date

    var isActive: Bool { state == .touching || state == .pressing }

    init(
        position: SIMD3<Double>,
        velocity: SIMD3<Double> = .zero,
        radius: Double,
        strength: Double,
        state: State,
        timestamp: Date = Date()
    ) {
        self.position = position
        self.velocity = velocity
        self.radius = max(0.0001, radius)
        self.strength = min(max(strength, 0), 1)
        self.state = state
        self.timestamp = timestamp
    }

    /// Bridge into the shared air-contact representation consumed by sculpt layers.
    func toAirContactField() -> MIRAirContactField {
        let airState: MIRAirContactField.State
        switch state {
        case .hovering: airState = .hovering
        case .touching: airState = .touching
        case .pressing: airState = .pressing
        case .released: airState = .released
        }
        return MIRAirContactField(
            position: position,
            velocity: velocity,
            radius: radius,
            strength: strength,
            state: airState,
            timestamp: timestamp
        )
    }
}
