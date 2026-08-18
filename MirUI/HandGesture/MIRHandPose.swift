import Foundation
import simd

/// Which hand a pose belongs to.
enum Handedness: String, Sendable {
    case left
    case right
    case unknown
}

/// A complete, camera-independent snapshot of one hand at a single moment.
/// The module never mutates scene geometry; it only describes the hand.
struct MIRHandPose: Sendable {
    let id: UUID
    let handedness: Handedness
    let landmarks: [MIRHandLandmark]

    let palmPosition: SIMD3<Double>
    let palmNormal: SIMD3<Double>

    let confidence: Double
    let timestamp: Date

    /// Convenience lookup of a single landmark by id.
    func landmark(_ id: LandmarkID) -> MIRHandLandmark? {
        landmarks.first { $0.id == id }
    }

    /// Normalised position of a landmark, or nil when not tracked.
    func point(_ id: LandmarkID) -> SIMD3<Double>? {
        landmark(id)?.normalizedPosition
    }
}
