import Foundation
import simd

// MARK: - Режим визуализации скелета

/// Отдельный режим визуализации скелета кистей в 3D (debug / assist).
/// Не смешивается с CAD-геометрией и не попадает в Document / History.
public enum MIRHandSkeletonVisMode: Int, Sendable, CaseIterable {
    case off = 0
    case jointsOnly = 1   // только суставы (points)
    case bones = 2        // суставы + кости (lines)
    case bonesAndRays = 3 // + указывающий луч
}

// MARK: - Топология костей

/// Статическая топология скелета (21 landmark, Vision / MediaPipe порядок).
/// Индексы костей в C++ (`HandSkeletonPass::kBoneIndices`) заданы в том же
/// порядке `LandmarkID.allCases`, что и здесь, — builder эмитит суставы именно
/// в этом порядке, поэтому растеризатор рисует правильные кости.
public enum MIRHandSkeletonTopology {
    /// Пары (parent, child) для линий «костей».
    public static let bones: [(LandmarkID, LandmarkID)] = [
        // запястье → основания пальцев
        (.wrist, .thumbCMC),
        (.wrist, .indexMCP),
        (.wrist, .middleMCP),
        (.wrist, .ringMCP),
        (.wrist, .littleMCP),
        // большой
        (.thumbCMC, .thumbMCP), (.thumbMCP, .thumbIP), (.thumbIP, .thumbTip),
        // указательный
        (.indexMCP, .indexPIP), (.indexPIP, .indexDIP), (.indexDIP, .indexTip),
        // средний
        (.middleMCP, .middlePIP), (.middlePIP, .middleDIP), (.middleDIP, .middleTip),
        // безымянный
        (.ringMCP, .ringPIP), (.ringPIP, .ringDIP), (.ringDIP, .ringTip),
        // мизинец
        (.littleMCP, .littlePIP), (.littlePIP, .littleDIP), (.littleDIP, .littleTip),
        // «ладонь» (опционально)
        (.indexMCP, .middleMCP), (.middleMCP, .ringMCP), (.ringMCP, .littleMCP)
    ]
}

// MARK: - Кадр скелета в 3D (scene space)

public struct MIRHandSkeletonFrame: Sendable {
    public struct Joint: Sendable {
        public let id: LandmarkID
        public let position: SIMD3<Double>  // scene space
        public let confidence: Double
    }

    public let handID: UUID
    public let handedness: Handedness
    public let joints: [Joint]             // в порядке LandmarkID.allCases (21)
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

// MARK: - Построение кадра

public enum MIRHandSkeletonBuilder {
    /// Строит кадр скелета из pose, маппя нормализованные landmark в scene space.
    /// Суставы эмитятся строго в порядке `LandmarkID.allCases`, чтобы индексы
    /// костей на стороне растера (C++) совпадали с позициями.
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
            // Отсутствующий landmark сохраняет выравнивание (позиция 0, conf 0).
            return MIRHandSkeletonFrame.Joint(id: id, position: .zero, confidence: 0)
        }
        return MIRHandSkeletonFrame(
            handID: pose.id,
            handedness: pose.handedness,
            joints: joints,
            gesture: gesture,
            pinch: pinch)
    }
}
