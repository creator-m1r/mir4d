import Foundation
import CoreGraphics
import simd

struct MIRHandSpatialMapper: Sendable {

    struct Volume: Equatable, Sendable {
        var width: Double = 2.0
        var height: Double = 1.25
        var depth: Double = 1.6

        var depthCenter: Double = 0.0
    }

    var volume: Volume = .init()

    var mirrorX: Bool = true

    func map(normalized position: SIMD3<Double>) -> SIMD3<Double> {
        let x = mirrorX ? 1.0 - position.x : position.x
        let y = 1.0 - position.y
        let z = min(max(position.z, -1), 1)
        return SIMD3(
            (x - 0.5) * volume.width,
            (y - 0.5) * volume.height,
            volume.depthCenter + z * volume.depth * 0.5
        )
    }

    func map(_ point: CGPoint, depth: Double = 0) -> SIMD3<Double> {
        map(normalized: SIMD3(Double(point.x), Double(point.y), depth))
    }

    func normalizedDistance(_ a: SIMD3<Double>, _ b: SIMD3<Double>) -> Double {
        simd_distance(a, b)
    }
}
