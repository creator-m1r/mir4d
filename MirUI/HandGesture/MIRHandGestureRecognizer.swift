import Foundation
import simd

/// A recognised gesture together with its lifecycle phase.
struct MIRHandGestureEvent: Sendable {
    let gesture: MIRHandGesture
    let phase: MIRHandGesturePhase
}

/// Stateful, single-hand gesture recognizer.
///
/// Responsibilities:
/// - temporal smoothing of position/velocity;
/// - confidence filtering (low-confidence frames are ignored);
/// - outlier rejection (implausible jumps are dropped);
/// - lost-frame tolerance (a few missing frames are tolerated);
/// - debounced gesture commitment (avoids spurious command bursts);
/// - gesture lifecycle phases: began → changed → ended/cancelled.
struct MIRHandGestureRecognizer: Sendable {
    struct Configuration: Sendable {
        var minConfidence: Double = 0.5
        var minHoldFrames: Int = 3
        var lostFrameTolerance: Int = 8
        var maxPositionJump: Double = 0.6
        var motion: MIRHandMotion.Configuration = .init()
        var classifying: MIRHandGestureClassifier.Configuration = .init()
    }

    var configuration = Configuration()

    private var classifier = MIRHandGestureClassifier()
    private var motion = MIRHandMotion()

    private var state: MIRHandState = .lost
    private var smoothedPosition: SIMD3<Double>?
    private var lastDirection: SIMD3<Double> = .zero
    private var lostFrames = 0

    private var pendingType: MIRHandGestureType = .rest
    private var pendingFrames = 0
    private var confirmedType: MIRHandGestureType = .rest
    private var confirmedConfidence: Double = 0
    private var lastEventTimestamp: Date = .distantPast

    var currentState: MIRHandState { state }
    var activeGesture: MIRHandGestureType { confirmedType }
    var currentVelocity: SIMD3<Double> { motion.currentVelocity }
    var currentDirection: SIMD3<Double> { lastDirection }

    /// Feed a freshly tracked pose (in normalised space) plus its scene-space
    /// palm position. Returns a gesture event when one should be emitted, else nil.
    mutating func ingest(pose: MIRHandPose, scenePosition: SIMD3<Double>, timestamp: Date) -> MIRHandGestureEvent? {
        classifier.configuration = configuration.classifying
        motion.configuration = configuration.motion

        // Confidence filtering + outlier rejection.
        guard pose.confidence >= configuration.minConfidence else {
            return handleMissing(timestamp: timestamp)
        }
        if let previous = smoothedPosition,
           simd_distance(previous, scenePosition) > configuration.maxPositionJump {
            // Treat as a dropped/outlier frame but keep tolerance alive.
            return handleMissing(timestamp: timestamp, penalise: false)
        }

        lostFrames = 0
        let a = configuration.motion.positionSmoothing
        let smoothed: SIMD3<Double>
        if let previous = smoothedPosition {
            smoothed = previous + (scenePosition - previous) * a
        } else {
            smoothed = scenePosition
        }
        smoothedPosition = smoothed

        let movement = motion.update(position: smoothed, timestamp: timestamp)
        lastDirection = movement.direction
        let result = classifier.classify(pose)

        let observed = true
        let interacting = [MIRHandGestureType.pinch, .grab, .point, .twoFinger, .threeFinger].contains(result.type)
        state = state.next(observed: observed, interacting: interacting)

        // Debounce: only commit after the candidate is stable for minHoldFrames.
        if result.type == pendingType {
            pendingFrames += 1
        } else {
            pendingType = result.type
            pendingFrames = 1
        }

        if pendingType != confirmedType && pendingFrames >= configuration.minHoldFrames {
            if confirmedType != .rest {
                _ = makeEvent(type: confirmedType, confidence: confirmedConfidence, strength: result.pinchStrength,
                              position: smoothed, movement: movement, timestamp: timestamp, phase: .ended)
            }
            confirmedType = pendingType
            confirmedConfidence = result.confidence
            return makeEvent(type: confirmedType, confidence: result.confidence, strength: result.pinchStrength,
                             position: smoothed, movement: movement, timestamp: timestamp, phase: .began)
        } else if confirmedType != .rest {
            confirmedConfidence = confirmedConfidence * 0.7 + result.confidence * 0.3
            return makeEvent(type: confirmedType, confidence: confirmedConfidence, strength: result.pinchStrength,
                             position: smoothed, movement: movement, timestamp: timestamp, phase: .changed)
        }

        return nil
    }

    /// Call when no pose arrived for this hand this frame.
    mutating func handleMissing(timestamp: Date, penalise: Bool = true) -> MIRHandGestureEvent? {
        if penalise { lostFrames += 1 }
        guard lostFrames > configuration.lostFrameTolerance else { return nil }

        let previous = confirmedType
        confirmedType = .rest
        confirmedConfidence = 0
        pendingType = .rest
        pendingFrames = 0
        smoothedPosition = nil
        motion.reset()
        state = state.next(observed: false, interacting: false)

        guard previous != .rest else { return nil }
        return MIRHandGestureEvent(
            gesture: MIRHandGesture(type: previous, confidence: confirmedConfidence, position: .zero, timestamp: timestamp),
            phase: .cancelled
        )
    }

    mutating func reset() {
        motion.reset()
        state = .lost
        smoothedPosition = nil
        lastDirection = .zero
        lostFrames = 0
        pendingType = .rest
        pendingFrames = 0
        confirmedType = .rest
        confirmedConfidence = 0
    }

    private func makeEvent(
        type: MIRHandGestureType,
        confidence: Double,
        strength: Double,
        position: SIMD3<Double>,
        movement: (velocity: SIMD3<Double>, acceleration: SIMD3<Double>, direction: SIMD3<Double>, speed: Double),
        timestamp: Date,
        phase: MIRHandGesturePhase
    ) -> MIRHandGestureEvent {
        MIRHandGestureEvent(
            gesture: MIRHandGesture(
                type: type,
                confidence: confidence,
                position: position,
                direction: movement.direction,
                velocity: movement.velocity,
                strength: strength,
                timestamp: timestamp
            ),
            phase: phase
        )
    }
}
