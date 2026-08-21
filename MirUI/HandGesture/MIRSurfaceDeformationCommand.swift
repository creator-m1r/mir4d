import Foundation
import simd

struct MIRSurfaceDeformationCommand: Equatable, Sendable {
    enum Mode: String, Sendable {
        case push
        case pull
        case smooth
    }

    let mode: Mode
    let center: SIMD3<Double>
    let normal: SIMD3<Double>
    let displacement: Double
    let radius: Double
    let strength: Double
    let falloff: Double
    let timestamp: Date

    init(
        mode: Mode,
        center: SIMD3<Double>,
        normal: SIMD3<Double>,
        displacement: Double,
        radius: Double,
        strength: Double,
        falloff: Double = 2.0,
        timestamp: Date = Date()
    ) {
        self.mode = mode
        self.center = center
        self.normal = simd_normalize(normal)
        self.displacement = displacement
        self.radius = max(0.0001, radius)
        self.strength = min(max(strength, 0), 1)
        self.falloff = max(0.1, falloff)
        self.timestamp = timestamp
    }

    func influence(at point: SIMD3<Double>) -> Double {
        let distance = simd_distance(point, center)
        guard distance < radius else { return 0 }
        let normalized = 1.0 - distance / radius
        return pow(normalized, falloff) * strength
    }

    func signedDisplacement(at point: SIMD3<Double>) -> Double {
        let influenceValue = influence(at: point)
        switch mode {
        case .push: return abs(displacement) * influenceValue
        case .pull: return -abs(displacement) * influenceValue
        case .smooth: return 0
        }
    }
}

@MainActor
final class MIRSurfaceDeformationBuilder {
    func makeCommand(from interaction: MIRSurfaceInteraction) -> MIRSurfaceDeformationCommand? {
        let mode: MIRSurfaceDeformationCommand.Mode
        switch interaction.operation {
        case .push: mode = .push
        case .pull: mode = .pull
        case .smooth: mode = .smooth
        case .hover: return nil
        }

        return MIRSurfaceDeformationCommand(
            mode: mode,
            center: interaction.point,
            normal: interaction.normal,
            displacement: interaction.displacement,
            radius: interaction.radius,
            strength: interaction.strength,
            timestamp: interaction.timestamp
        )
    }
}
