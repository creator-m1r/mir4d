import Foundation
import CoreGraphics
import simd

/// Maps normalised camera-hand coordinates into a stable 3D interaction volume.
///
/// The module deliberately does **not** hard-code CAD scene dimensions: the
/// mapping volume is configurable and consumed by the hand module only. CAD
/// layers decide how to interpret the resulting scene-space coordinates.
struct MIRHandSpatialMapper: Sendable {
    /// The interaction volume the normalised hand space is projected into.
    struct Volume: Equatable, Sendable {
        var width: Double = 2.0
        var height: Double = 1.25
        var depth: Double = 1.6
        /// Centre of the volume along the depth axis.
        var depthCenter: Double = 0.0
    }

    var volume: Volume = .init()
    /// Mirror the X axis (camera sees a mirrored user by default).
    var mirrorX: Bool = true

    /// Project a normalised point (x,y ∈ [0,1], depth ∈ [-1,1]) into scene space.
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

    /// Convenience for a CGPoint with an explicit depth.
    func map(_ point: CGPoint, depth: Double = 0) -> SIMD3<Double> {
        map(normalized: SIMD3(Double(point.x), Double(point.y), depth))
    }

    /// Normalised distance between two normalised points (resolution independent).
    func normalizedDistance(_ a: SIMD3<Double>, _ b: SIMD3<Double>) -> Double {
        simd_distance(a, b)
    }
}
