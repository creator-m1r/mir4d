import Foundation
import CoreGraphics

/// Maps normalized camera-hand coordinates into a stable 3D interaction volume.
/// This layer does not mutate scene geometry.
struct MIRHandSpatialMapper: Sendable {
    struct Volume: Equatable, Sendable {
        var width: Double = 2.0
        var height: Double = 1.25
        var depth: Double = 1.6
        var depthCenter: Double = 0.0
    }

    var volume = Volume()
    var mirrorX = true

    func map(normalized point: CGPoint, depth: Double = 0) -> SIMD3<Double> {
        let x = mirrorX ? 1.0 - Double(point.x) : Double(point.x)
        let y = 1.0 - Double(point.y)
        let z = min(max(depth, -1), 1)
        return SIMD3(
            (x - 0.5) * volume.width,
            (y - 0.5) * volume.height,
            volume.depthCenter + z * volume.depth * 0.5
        )
    }

    func normalizedDistance(_ a: CGPoint, _ b: CGPoint) -> Double {
        hypot(Double(a.x - b.x), Double(a.y - b.y))
    }
}
