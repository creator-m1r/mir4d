import Foundation
import CoreGraphics
import simd
import simd

/// Converts spatial hand samples into a continuous air-sculpt stroke.
/// This layer owns interaction state only; mesh mutation belongs to MirEngine.
@MainActor
final class MIRAirSculptController {
    enum Mode: Equatable {
        case sculpt
        case grab
        case draw
    }

    enum State: Equatable {
        case idle
        case hovering
        case contact
        case sculpting
        case released
    }

    struct Configuration: Equatable, Sendable {
        var contactDistance: Double = 0.045
        var releaseDistance: Double = 0.075
        var minimumSampleDistance: Double = 0.004
        var defaultRadius: Double = 0.025
        var minimumRadius: Double = 0.005
        var maximumRadius: Double = 0.15
        var strengthScale: Double = 1.0
    }

    struct Sample: Equatable, Sendable {
        let position: SIMD3<Double>
        let normal: SIMD3<Double>?
        let radius: Double
        let strength: Double
        let timestamp: Date
    }

    private(set) var state: State = .idle
    private(set) var configuration = Configuration()
    private(set) var samples: [Sample] = []
    private var lastPosition: SIMD3<Double>?

    func update(configuration: Configuration) {
        self.configuration = configuration
    }

    func reset() {
        state = .idle
        samples.removeAll()
        lastPosition = nil
    }

    func updateHand(position: SIMD3<Double>, distanceToSurface: Double, pinch: Double, timestamp: Date = Date(), surfaceNormal: SIMD3<Double>? = nil) -> Sample? {
        if distanceToSurface > configuration.releaseDistance {
            state = .hovering
            lastPosition = position
            return nil
        }

        if distanceToSurface > configuration.contactDistance {
            state = .hovering
            lastPosition = position
            return nil
        }

        state = .contact
        let movement = lastPosition.map { simd_distance($0, position) } ?? 0
        guard movement >= configuration.minimumSampleDistance else { return nil }

        state = .sculpting
        lastPosition = position

        let radius = max(configuration.minimumRadius, min(configuration.maximumRadius, configuration.defaultRadius * (0.5 + pinch * 2.0)))
        let strength = max(0, min(1, pinch)) * configuration.strengthScale
        let sample = Sample(position: position, normal: surfaceNormal, radius: radius, strength: strength, timestamp: timestamp)
        samples.append(sample)
        if samples.count > 4096 { samples.removeFirst(samples.count - 4096) }
        return sample
    }

    func release() {
        state = .released
        lastPosition = nil
    }
}
