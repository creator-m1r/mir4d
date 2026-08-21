import Foundation
import simd

public enum MIRHandSkeletonVisMode: Int, Sendable, CaseIterable {
    case off = 0
    case jointsOnly = 1
    case bones = 2
    case bonesAndRays = 3
}

public enum MIRHandSkeletonTopology {

    public static let bones: [(LandmarkID, LandmarkID)] = [

        (.wrist, .thumbCMC),
        (.wrist, .indexMCP),
        (.wrist, .middleMCP),
        (.wrist, .ringMCP),
        (.wrist, .littleMCP),

        (.thumbCMC, .thumbMCP), (.thumbMCP, .thumbIP), (.thumbIP, .thumbTip),

        (.indexMCP, .indexPIP), (.indexPIP, .indexDIP), (.indexDIP, .indexTip),

        (.middleMCP, .middlePIP), (.middlePIP, .middleDIP), (.middleDIP, .middleTip),

        (.ringMCP, .ringPIP), (.ringPIP, .ringDIP), (.ringDIP, .ringTip),

        (.littleMCP, .littlePIP), (.littlePIP, .littleDIP), (.littleDIP, .littleTip),

        (.indexMCP, .middleMCP), (.middleMCP, .ringMCP), (.ringMCP, .littleMCP)
    ]
}

public struct MIRHandSkeletonFrame: Sendable {
    public struct Joint: Sendable {
        public let id: LandmarkID
        public let position: SIMD3<Double>
        public let confidence: Double
    }

    public let handID: UUID
    public let handedness: Handedness
    public let joints: [Joint]
    public let gesture: MIRHandGestureType
    public let pinch: Double

    public init(handID: UUID, handedness: Handedness, joints: [Joint],
               gesture: MIRHandGestureType, pinch: Double) {
        self.handID = handID
        self.handedness = handedness
        self.joints = joints
        self.gesture = gesture
        self.pinch = pinch
    }
}

public enum MIRHandSkeletonBuilder {

    static func build(
        pose: MIRHandPose,
        mapper: MIRHandSpatialMapper,
        gesture: MIRHandGestureType = .rest,
        pinch: Double = 0
    ) -> MIRHandSkeletonFrame {
        let byId = Dictionary(uniqueKeysWithValues: pose.landmarks.map { ($0.id, $0) })
        let joints = LandmarkID.allCases.map { id -> MIRHandSkeletonFrame.Joint in
            if let lm = byId[id] {
                return MIRHandSkeletonFrame.Joint(
                    id: id,
                    position: mapper.map(normalized: lm.normalizedPosition),
                    confidence: lm.confidence)
            }

            return MIRHandSkeletonFrame.Joint(id: id, position: .zero, confidence: 0)
        }
        return MIRHandSkeletonFrame(
            handID: pose.id,
            handedness: pose.handedness,
            joints: joints,
            gesture: gesture,
            pinch: pinch)
    }

    public static func topologyIndices() -> [Int32] {
        MIRHandSkeletonTopology.bones.flatMap { (a, b) in
            [Int32(LandmarkID.allCases.firstIndex(of: a)!),
             Int32(LandmarkID.allCases.firstIndex(of: b)!)]
        }
    }

    public static func gestureCode(_ gesture: MIRHandGestureType) -> Int32 {
        Int32(MIRHandGestureType.allCases.firstIndex(of: gesture) ?? 0)
    }
}
