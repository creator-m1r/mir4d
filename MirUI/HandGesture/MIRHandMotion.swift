import Foundation
import simd

struct MIRHandMotion: Sendable {
    struct Configuration: Sendable {
        var historyLength: Int = 12
        var positionSmoothing: Double = 0.35
        var velocitySmoothing: Double = 0.4
    }

    var configuration = Configuration()

    private var history: [(position: SIMD3<Double>, timestamp: Date)] = []
    private var smoothedPosition: SIMD3<Double>?
    private var smoothedVelocity: SIMD3<Double> = .zero

    mutating func reset() {
        history.removeAll()
        smoothedPosition = nil
        smoothedVelocity = .zero
    }

    mutating func update(position: SIMD3<Double>, timestamp: Date) -> (
        velocity: SIMD3<Double>, acceleration: SIMD3<Double>, direction: SIMD3<Double>, speed: Double
    ) {
        let a = configuration.positionSmoothing
        let smoothed: SIMD3<Double>
        if let previous = smoothedPosition {
            smoothed = previous + (position - previous) * a
        } else {
            smoothed = position
        }
        smoothedPosition = smoothed

        history.append((smoothed, timestamp))
        if history.count > configuration.historyLength {
            history.removeFirst(history.count - configuration.historyLength)
        }

        let velocity: SIMD3<Double>
        if history.count >= 2,
           let prev = history.dropLast().last {
            let dt = max(timestamp.timeIntervalSince(prev.timestamp), 1.0 / 120.0)
            let instantaneous = (smoothed - prev.position) / dt
            let va = configuration.velocitySmoothing
            velocity = smoothedVelocity + (instantaneous - smoothedVelocity) * va
        } else {
            velocity = .zero
        }
        smoothedVelocity = velocity

        let acceleration: SIMD3<Double>
        if history.count >= 3,
           let prev = history.dropLast().last,
           let before = history.dropLast(2).last {
            let dt1 = max(prev.timestamp.timeIntervalSince(before.timestamp), 1.0 / 120.0)
            let dt2 = max(timestamp.timeIntervalSince(prev.timestamp), 1.0 / 120.0)
            let v1 = (prev.position - before.position) / dt1
            let v2 = (smoothed - prev.position) / dt2
            acceleration = (v2 - v1) / max(dt2, 1.0 / 120.0)
        } else {
            acceleration = .zero
        }

        let speed = simd_length(velocity)
        let direction = speed > 1e-6 ? simd_normalize(velocity) : .zero
        return (velocity, acceleration, direction, speed)
    }

    var currentVelocity: SIMD3<Double> { smoothedVelocity }
}
