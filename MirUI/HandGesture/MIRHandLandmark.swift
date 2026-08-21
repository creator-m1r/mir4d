import Foundation
import simd

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

struct MIRHandLandmark: Sendable {
    let id: LandmarkID
    let normalizedPosition: SIMD3<Double>
    let confidence: Double
}

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

    var tip: LandmarkID { joints.last! }

    var mcp: LandmarkID { joints[1] }
}
