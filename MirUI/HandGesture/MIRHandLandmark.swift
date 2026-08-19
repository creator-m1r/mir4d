import Foundation
import simd

/// Identifier of a single tracked hand joint.
/// Mirrors the canonical 21-point hand skeleton used by platform hand APIs.
public enum LandmarkID: String, Sendable, CaseIterable, Hashable {
    case wrist

    case thumbCMC
    case thumbMCP
    case thumbIP
    case thumbTip

    case indexMCP
    case indexPIP
    case indexDIP
    case indexTip

    case middleMCP
    case middlePIP
    case middleDIP
    case middleTip

    case ringMCP
    case ringPIP
    case ringDIP
    case ringTip

    case littleMCP
    case littlePIP
    case littleDIP
    case littleTip
}

/// A single recognised joint of a hand.
/// `normalizedPosition` lives in normalised camera space:
/// x ∈ [0,1] left→right, y ∈ [0,1] top→bottom, z is relative depth (roughly [−1,1]).
struct MIRHandLandmark: Sendable {
    let id: LandmarkID
    let normalizedPosition: SIMD3<Double>
    let confidence: Double
}

/// A logical finger of the hand.
enum MIRFinger: Sendable, CaseIterable {
    case thumb
    case index
    case middle
    case ring
    case little

    var joints: [LandmarkID] {
        switch self {
        case .thumb:  [.thumbCMC, .thumbMCP, .thumbIP, .thumbTip]
        case .index:  [.indexMCP, .indexPIP, .indexDIP, .indexTip]
        case .middle: [.middleMCP, .middlePIP, .middleDIP, .middleTip]
        case .ring:   [.ringMCP, .ringPIP, .ringDIP, .ringTip]
        case .little: [.littleMCP, .littlePIP, .littleDIP, .littleTip]
        }
    }

    /// Tip joint for the finger.
    var tip: LandmarkID { joints.last! }

    /// Metacarpophalangeal joint (knuckle) for the finger.
    var mcp: LandmarkID { joints[1] }
}
