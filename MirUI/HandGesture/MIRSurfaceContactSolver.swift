import Foundation
import simd

/// Resolves an air-contact sphere against a renderable surface supplied by the CAD layer.
/// The solver does not mutate geometry.
@MainActor
final class MIRSurfaceContactSolver {
    struct Hit: Equatable, Sendable {
        let point: SIMD3<Double>
        let normal: SIMD3<Double>
        let distance: Double
    }

    protocol SurfaceQuery {
        func nearestSurface(to point: SIMD3<Double>, maxDistance: Double) -> Hit?
    }

    func resolve(field: MIRAirContactField, query: SurfaceQuery) -> MIRAirContactPreview {
        let hit = query.nearestSurface(to: field.position, maxDistance: field.radius * 3)
        return MIRAirContactPreview(
            field: field,
            surfacePoint: hit?.point,
            surfaceNormal: hit?.normal,
            distanceToSurface: hit?.distance
        )
    }
}
