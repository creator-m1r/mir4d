import Foundation
import simd

public enum Handedness: String, Sendable {
    case left
    case right
    case unknown
}

struct MIRHandPose: Sendable {
    let id: UUID
    let handedness: Handedness
    let landmarks: [MIRHandLandmark]

    let palmPosition: SIMD3<Double>
    let palmNormal: SIMD3<Double>

    let confidence: Double
    let timestamp: Date

    func landmark(_ id: LandmarkID) -> MIRHandLandmark? {
        landmarks.first { $0.id == id }
    }

    func point(_ id: LandmarkID) -> SIMD3<Double>? {
        landmark(id)?.normalizedPosition
    }
}
