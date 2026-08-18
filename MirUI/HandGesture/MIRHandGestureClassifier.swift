import Foundation
import simd

/// Pure, side-effect-free gesture classification.
///
/// It derives finger curls, inter-finger distances, normalised ratios and a
/// continuous `pinchStrength`, then selects the best matching gesture together
/// with a confidence in `0...1`. The classifier is intentionally data-driven
/// and extensible: add new rules without touching the rest of the pipeline.
struct MIRHandGestureClassifier: Sendable {
    struct Configuration: Sendable {
        /// Normalised distance (relative to palm size) at which thumb↔index is "fully pinched".
        var pinchScale: Double = 0.45
        /// Curl value at/above which a finger counts as "folded".
        var curlFolded: Double = 0.6
        /// Curl value at/below which a finger counts as "extended".
        var curlExtended: Double = 0.35
        /// Curl value at/above which a finger counts as "fully closed" (fist).
        var fistFolded: Double = 0.8
    }

    var configuration = Configuration()

    // MARK: - Geometry helpers

    private func joint(_ pose: MIRHandPose, _ id: LandmarkID) -> SIMD3<Double>? {
        pose.point(id)
    }

    /// Curl ∈ [0,1]: 0 = straight, 1 = fully folded. Distance-independent.
    func curl(of finger: MIRFinger, in pose: MIRHandPose) -> Double {
        let joints = finger.joints.compactMap { joint(pose, $0) }
        guard joints.count == finger.joints.count else { return 0 }
        let tip = joints.last!
        let mcp = joints[1]
        let tipToMcp = simd_distance(tip, mcp)
        let segmentSum = zip(joints, joints.dropFirst()).map { simd_distance($0, $1) }.reduce(0, +)
        guard segmentSum > 1e-6 else { return 0 }
        return min(max(1 - tipToMcp / segmentSum, 0), 1)
    }

    /// Continuous pinch strength ∈ [0,1]: 0 = fingers far apart, 1 = fully pinched.
    func pinchStrength(in pose: MIRHandPose) -> Double {
        guard let thumb = joint(pose, .thumbTip),
              let index = joint(pose, .indexTip),
              let wrist = joint(pose, .wrist),
              let middleMCP = joint(pose, .middleMCP) else { return 0 }
        let scale = max(simd_distance(wrist, middleMCP), 1e-4)
        let d = simd_distance(thumb, index)
        return min(max(1 - d / (scale * configuration.pinchScale), 0), 1)
    }

    /// Adjacent-finger tip distances (normalised by palm size).
    func interFingerDistances(in pose: MIRHandPose) -> (
        thumbIndex: Double, indexMiddle: Double, middleRing: Double, ringLittle: Double
    ) {
        let scale = max(pose.point(.middleMCP).map { simd_distance($0, pose.palmPosition) } ?? 1e-4, 1e-4)
        func d(_ a: LandmarkID, _ b: LandmarkID) -> Double {
            guard let pa = pose.point(a), let pb = pose.point(b) else { return 0 }
            return simd_distance(pa, pb) / scale
        }
        return (
            d(.thumbTip, .indexTip),
            d(.indexTip, .middleTip),
            d(.middleTip, .ringTip),
            d(.ringTip, .littleTip)
        )
    }

    // MARK: - Classification

    struct Result: Sendable {
        let type: MIRHandGestureType
        let confidence: Double
        let curls: [MIRFinger: Double]
        let pinchStrength: Double
        let extendedFingers: Int
    }

    func classify(_ pose: MIRHandPose) -> Result {
        let curls = Dictionary(uniqueKeysWithValues: MIRFinger.allCases.map { ($0, curl(of: $0, in: pose)) })
        let pinch = pinchStrength(in: pose)
        let extended = MIRFinger.allCases.filter { curls[$0]! <= configuration.curlExtended }.count

        var best: MIRHandGestureType = .rest
        var bestScore: Double = 0

        func consider(_ type: MIRHandGestureType, _ score: Double) {
            if score > bestScore { bestScore = score; best = type }
        }

        // PINCH: thumb and index close, index reasonably extended.
        if pinch > 0.7 && curls[.index]! <= 0.6 {
            consider(.pinch, 0.6 + pinch * 0.4)
        }

        // FIST: everything folded tightly.
        if extended == 0 && curls.values.allSatisfy({ $0 >= configuration.fistFolded }) {
            consider(.fist, 0.85)
        }

        // GRAB: fingers folded (as if holding) but a touch looser than a fist.
        if extended <= 1 && curls.values.filter({ $0 >= configuration.curlFolded }).count >= 4 {
            consider(.grab, 0.8)
        }

        // OPEN_PALM: all fingers extended and spread.
        if extended == 5 {
            let spread = interFingerDistances(in: pose)
            let spreadScore = min((spread.thumbIndex + spread.indexMiddle + spread.middleRing + spread.ringLittle) / 4, 1)
            consider(.openPalm, 0.7 + spreadScore * 0.25)
        }

        // POINT: only index extended.
        if curls[.index]! <= configuration.curlExtended &&
           curls[.middle]! >= configuration.curlFolded &&
           curls[.ring]! >= configuration.curlFolded &&
           curls[.little]! >= configuration.curlFolded {
            consider(.point, 0.9)
        }

        // TWO_FINGER: index + middle extended.
        if curls[.index]! <= configuration.curlExtended &&
           curls[.middle]! <= configuration.curlExtended &&
           curls[.ring]! >= configuration.curlFolded &&
           curls[.little]! >= configuration.curlFolded {
            consider(.twoFinger, 0.88)
        }

        // THREE_FINGER: index + middle + ring extended.
        if curls[.index]! <= configuration.curlExtended &&
           curls[.middle]! <= configuration.curlExtended &&
           curls[.ring]! <= configuration.curlExtended &&
           curls[.little]! >= configuration.curlFolded {
            consider(.threeFinger, 0.85)
        }

        // V_SIGN: index + middle extended and spread apart.
        if curls[.index]! <= configuration.curlExtended &&
           curls[.middle]! <= configuration.curlExtended &&
           interFingerDistances(in: pose).indexMiddle > 0.9 {
            consider(.vSign, 0.85)
        }

        // THUMBS_UP: thumb extended, other four folded.
        if curls[.thumb]! <= configuration.curlExtended &&
           curls[.index]! >= configuration.curlFolded &&
           curls[.middle]! >= configuration.curlFolded &&
           curls[.ring]! >= configuration.curlFolded &&
           curls[.little]! >= configuration.curlFolded {
            consider(.thumbsUp, 0.9)
        }

        if bestScore < 0.5 { best = .rest; bestScore = max(bestScore, 0.4) }

        return Result(
            type: best,
            confidence: min(max(bestScore, 0), 1),
            curls: curls,
            pinchStrength: pinch,
            extendedFingers: extended
        )
    }
}
