import Foundation
import simd

/// Represents the virtual contact volume produced by an engineer's hand.
/// It is independent of rendering and geometry mutation.
struct MIRAirContactField: Equatable, Sendable {
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

    var isActive: Bool {
        state == .touching || state == .pressing
    }

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
}

/// Maps a hand point from normalized camera space into the CAD interaction volume.
@MainActor
final class MIRAirContactFieldBuilder {
    struct Configuration: Equatable, Sendable {
        var sceneWidth: Double = 1.6
        var sceneHeight: Double = 1.0
        var depth: Double = 1.2
        var smoothing: Double = 0.18
        var defaultRadius: Double = 0.025
    }

    private(set) var configuration = Configuration()
    private var previousPosition: SIMD3<Double>?
    private var previousTimestamp: Date?

    func update(configuration: Configuration) {
        self.configuration = configuration
    }

    func reset() {
        previousPosition = nil
        previousTimestamp = nil
    }

    func makeField(
        normalizedX: Double,
        normalizedY: Double,
        normalizedDepth: Double,
        radius: Double? = nil,
        strength: Double,
        state: MIRAirContactField.State,
        timestamp: Date = Date()
    ) -> MIRAirContactField {
        let raw = SIMD3(
            (normalizedX - 0.5) * configuration.sceneWidth,
            (0.5 - normalizedY) * configuration.sceneHeight,
            (normalizedDepth - 0.5) * configuration.depth
        )

        let position: SIMD3<Double>
        if let previousPosition {
            let a = configuration.smoothing
            position = previousPosition + (raw - previousPosition) * a
        } else {
            position = raw
        }

        let velocity: SIMD3<Double>
        if let previousPosition, let previousTimestamp {
            let dt = max(timestamp.timeIntervalSince(previousTimestamp), 1.0 / 120.0)
            velocity = (position - previousPosition) / dt
        } else {
            velocity = .zero
        }

        self.previousPosition = position
        self.previousTimestamp = timestamp

        return MIRAirContactField(
            position: position,
            velocity: velocity,
            radius: radius ?? configuration.defaultRadius,
            strength: strength,
            state: state,
            timestamp: timestamp
        )
    }
}
