import Foundation
import simd

struct MIRSurfaceInteraction: Equatable, Sendable {
    enum Operation: String, Sendable {
        case hover
        case push
        case pull
        case smooth
    }

    let operation: Operation
    let point: SIMD3<Double>
    let normal: SIMD3<Double>
    let displacement: Double
    let radius: Double
    let strength: Double
    let timestamp: Date
}

@MainActor
final class MIRSurfaceInteractionBuilder {
    func makeInteraction(
        preview: MIRAirContactPreview,
        previousPosition: SIMD3<Double>?,
        mode: MIRAirSculptController.Mode = .sculpt
    ) -> MIRSurfaceInteraction? {
        guard let point = preview.surfacePoint, let normal = preview.surfaceNormal else { return nil }

        let displacement: Double
        if let previousPosition {
            displacement = simd_dot(preview.field.position - previousPosition, normal)
        } else {
            displacement = 0
        }

        let operation: MIRSurfaceInteraction.Operation
        switch mode {
        case .sculpt, .grab:
            operation = displacement >= 0 ? .push : .pull
        case .draw:
            operation = .smooth
        }

        return MIRSurfaceInteraction(
            operation: operation,
            point: point,
            normal: normal,
            displacement: displacement,
            radius: preview.field.radius,
            strength: preview.field.strength,
            timestamp: preview.field.timestamp
        )
    }
}
