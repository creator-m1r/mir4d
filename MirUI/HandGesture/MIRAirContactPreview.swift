import Foundation
import simd

struct MIRAirContactPreview: Equatable, Sendable {
    let field: MIRAirContactField
    let surfacePoint: SIMD3<Double>?
    let surfaceNormal: SIMD3<Double>?
    let distanceToSurface: Double?

    var isTouchingSurface: Bool {
        guard let distanceToSurface else { return false }
        return distanceToSurface <= field.radius
    }
}
