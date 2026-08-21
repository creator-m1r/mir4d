import Foundation
import simd

enum MIRHandPoseMock {

    private static func landmark(_ id: LandmarkID, _ p: SIMD3<Double>) -> MIRHandLandmark {
        MIRHandLandmark(id: id, normalizedPosition: p, confidence: 0.95)
    }

    private static func rotate(_ v: SIMD3<Double>, by angle: Double) -> SIMD3<Double> {
        let c = cos(angle), s = sin(angle)
        return SIMD3(v.x * c - v.y * s, v.x * s + v.y * c, 0)
    }

    private static func fingerLandmarks(finger: MIRFinger, mcp: SIMD3<Double>, up: SIMD3<Double>, curl: Double) -> [MIRHandLandmark] {
        let s1 = 0.05, s2 = 0.05, s3 = 0.05
        let u = simd_normalize(up)
        let phi = curl * 6.0
        let j0 = mcp
        let j1 = j0 + u * s1
        let j2 = j1 + rotate(u, by: phi * 0.5) * s2
        let j3 = j2 + rotate(u, by: phi) * s3
        let joints = finger.joints
        return [
            landmark(joints[0], j0),
            landmark(joints[1], j1),
            landmark(joints[2], j2),
            landmark(joints[3], j3)
        ]
    }

    static func pose(
        handedness: Handedness = .right,
        curls: [MIRFinger: Double],
        pinch: Double = 0.0
    ) -> MIRHandPose {
        var landmarks: [MIRHandLandmark] = []
        let wrist = SIMD3(0.5, 0.82, 0)
        landmarks.append(landmark(.wrist, wrist))

        let defs: [(MIRFinger, SIMD3<Double>, SIMD3<Double>)] = [
            (.index,  SIMD3(0.44, 0.62, 0), SIMD3(-0.12, -1, 0)),
            (.middle, SIMD3(0.50, 0.60, 0), SIMD3(0, -1, 0)),
            (.ring,   SIMD3(0.57, 0.62, 0), SIMD3(0.12, -1, 0)),
            (.little, SIMD3(0.64, 0.65, 0), SIMD3(0.22, -1, 0)),
            (.thumb,  SIMD3(0.40, 0.74, 0), SIMD3(-1, -0.2, 0))
        ]

        var indexTip: SIMD3<Double>?
        for (finger, mcpPos, up) in defs {
            let curl = curls[finger] ?? 0
            let fl = fingerLandmarks(finger: finger, mcp: mcpPos, up: up, curl: curl)
            if finger == .index { indexTip = fl.last?.normalizedPosition }
            landmarks.append(contentsOf: fl)
        }

        if pinch > 0, let it = indexTip,
           let idx = landmarks.firstIndex(where: { $0.id == .thumbTip }) {
            let natural = landmarks[idx].normalizedPosition
            let mixed = SIMD3(
                natural.x + (it.x - natural.x) * pinch,
                natural.y + (it.y - natural.y) * pinch,
                0
            )
            landmarks[idx] = MIRHandLandmark(id: .thumbTip, normalizedPosition: mixed, confidence: 0.95)
        }

        return MIRHandPose(
            id: UUID(),
            handedness: handedness,
            landmarks: landmarks,
            palmPosition: SIMD3(0.5, 0.66, 0),
            palmNormal: SIMD3(0, 0, 1),
            confidence: 0.95,
            timestamp: Date()
        )
    }

    static func mockPoint(handedness: Handedness = .right) -> MIRHandPose {
        pose(handedness: handedness, curls: [.index: 0.1, .middle: 0.8, .ring: 0.8, .little: 0.8, .thumb: 0.1])
    }

    static func mockPinch(handedness: Handedness = .right) -> MIRHandPose {
        pose(handedness: handedness, curls: [.index: 0.5, .middle: 0.8, .ring: 0.8, .little: 0.8, .thumb: 0.5], pinch: 0.9)
    }

    static func mockGrab(handedness: Handedness = .right) -> MIRHandPose {
        pose(handedness: handedness, curls: [.index: 0.65, .middle: 0.65, .ring: 0.65, .little: 0.65, .thumb: 0.65])
    }

    static func mockFist(handedness: Handedness = .right) -> MIRHandPose {
        pose(handedness: handedness, curls: [.index: 0.85, .middle: 0.85, .ring: 0.85, .little: 0.85, .thumb: 0.85])
    }

    static func mockOpenPalm(handedness: Handedness = .right) -> MIRHandPose {
        pose(handedness: handedness, curls: [.index: 0.1, .middle: 0.1, .ring: 0.1, .little: 0.1, .thumb: 0.1])
    }

    static func mockTwoFinger(handedness: Handedness = .right) -> MIRHandPose {
        pose(handedness: handedness, curls: [.index: 0.1, .middle: 0.1, .ring: 0.8, .little: 0.8, .thumb: 0.8])
    }

    static func mockThreeFinger(handedness: Handedness = .right) -> MIRHandPose {
        pose(handedness: handedness, curls: [.index: 0.1, .middle: 0.1, .ring: 0.1, .little: 0.8, .thumb: 0.8])
    }

    static func mockThumbsUp(handedness: Handedness = .right) -> MIRHandPose {
        pose(handedness: handedness, curls: [.index: 0.8, .middle: 0.8, .ring: 0.8, .little: 0.8, .thumb: 0.1])
    }

    static func mockRest(handedness: Handedness = .right) -> MIRHandPose {
        pose(handedness: handedness, curls: [.index: 0.5, .middle: 0.5, .ring: 0.5, .little: 0.5, .thumb: 0.5])
    }

    static func openPalmAt(center: SIMD3<Double>, handedness: Handedness) -> MIRHandPose {
        let base = mockOpenPalm(handedness: handedness)
        let delta = center - base.palmPosition
        let landmarks = base.landmarks.map {
            MIRHandLandmark(id: $0.id, normalizedPosition: $0.normalizedPosition + delta, confidence: $0.confidence)
        }
        return MIRHandPose(
            id: UUID(),
            handedness: handedness,
            landmarks: landmarks,
            palmPosition: center,
            palmNormal: base.palmNormal,
            confidence: base.confidence,
            timestamp: Date()
        )
    }

    static func mockTwoHandScale() -> [[MIRHandPose]] {
        let distances = [0.10, 0.14, 0.18, 0.14, 0.10]
        return distances.map { d in
            [
                openPalmAt(center: SIMD3(0.5 - d, 0.66, 0), handedness: .left),
                openPalmAt(center: SIMD3(0.5 + d, 0.66, 0), handedness: .right)
            ]
        }
    }

    static func mockTwoHandPair(leftCenter: SIMD3<Double>, rightCenter: SIMD3<Double>) -> [MIRHandPose] {
        [openPalmAt(center: leftCenter, handedness: .left),
         openPalmAt(center: rightCenter, handedness: .right)]
    }
}
