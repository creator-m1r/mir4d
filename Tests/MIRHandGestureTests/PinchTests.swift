import XCTest
@testable import MirUIHandGesture

final class PinchTests: XCTestCase {
    func testPinchIsRecognised() {
        let pose = MIRHandPoseMock.mockPinch()
        let result = MIRHandGestureClassifier().classify(pose)
        XCTAssertEqual(result.type, .pinch, "Mock pinch should classify as PINCH")
        XCTAssertGreaterThan(result.pinchStrength, 0.7)
    }

    func testPinchStrengthIsContinuous() {
        let far = MIRHandPoseMock.pose(curls: [.index: 0.5, .thumb: 0.5], pinch: 0.0)
        let mid = MIRHandPoseMock.pose(curls: [.index: 0.5, .thumb: 0.5], pinch: 0.5)
        let close = MIRHandPoseMock.pose(curls: [.index: 0.5, .thumb: 0.5], pinch: 1.0)
        let c = MIRHandGestureClassifier()
        XCTAssertLessThan(c.pinchStrength(in: far), c.pinchStrength(in: mid))
        XCTAssertLessThan(c.pinchStrength(in: mid), c.pinchStrength(in: close))
    }

    func testPinchIsRobustToNoise() {
        var recognizer = MIRHandGestureRecognizer()
        let mapper = MIRHandSpatialMapper()
        var committed: [MIRHandGestureEvent] = []
        for _ in 0..<10 {
            var pose = MIRHandPoseMock.mockPinch()
            // Add small jitter to one landmark to simulate camera noise.
            if let idx = pose.landmarks.firstIndex(where: { $0.id == .middleTip }) {
                let jittered = SIMD3(
                    pose.landmarks[idx].normalizedPosition.x + Double.random(in: -0.005...0.005),
                    pose.landmarks[idx].normalizedPosition.y, 0)
                var l = pose.landmarks
                l[idx] = MIRHandLandmark(id: .middleTip, normalizedPosition: jittered, confidence: 0.95)
                pose = MIRHandPose(id: pose.id, handedness: pose.handedness, landmarks: l,
                                   palmPosition: pose.palmPosition, palmNormal: pose.palmNormal,
                                   confidence: pose.confidence, timestamp: Date())
            }
            if let ev = recognizer.ingest(pose: pose, scenePosition: mapper.map(normalized: pose.palmPosition), timestamp: Date()) {
                committed.append(ev)
            }
        }
        XCTAssertEqual(recognizer.activeGesture, .pinch, "Noise must not break pinch tracking")
        XCTAssertFalse(committed.isEmpty, "A pinch gesture should be committed")
    }

    func testLowPinchDoesNotTrigger() {
        let pose = MIRHandPoseMock.pose(curls: [.index: 0.5, .thumb: 0.5], pinch: 0.0)
        XCTAssertNotEqual(MIRHandGestureClassifier().classify(pose).type, .pinch)
    }

    func testPinchStrengthIsSmoothed() {
        var recognizer = MIRHandGestureRecognizer()
        let mapper = MIRHandSpatialMapper()
        let strong = MIRHandPoseMock.pose(curls: [.index: 0.5, .middle: 0.8, .ring: 0.8, .little: 0.8, .thumb: 0.5], pinch: 1.0)
        let weak = MIRHandPoseMock.pose(curls: [.index: 0.5, .middle: 0.8, .ring: 0.8, .little: 0.8, .thumb: 0.5], pinch: 0.75)

        // Warm up so the pinch gesture is committed.
        for _ in 0..<8 {
            _ = recognizer.ingest(pose: strong, scenePosition: mapper.map(normalized: strong.palmPosition), timestamp: Date())
        }

        // Alternate strong/weak; raw strength would swing ~0.25 each frame.
        var emitted: [Double] = []
        for i in 0..<10 {
            let p = (i % 2 == 0) ? strong : weak
            if let ev = recognizer.ingest(pose: p, scenePosition: mapper.map(normalized: p.palmPosition), timestamp: Date()),
               ev.phase == .changed {
                emitted.append(ev.gesture.strength)
            }
        }
        guard emitted.count >= 2 else {
            XCTFail("expected smoothed strength samples")
            return
        }
        var maxDelta = 0.0
        for i in 1..<emitted.count {
            maxDelta = max(maxDelta, abs(emitted[i] - emitted[i - 1]))
        }
        // EMA(0.4) must damp the raw 0.25 swing substantially.
        XCTAssertLessThan(maxDelta, 0.2, "smoothed pinch strength must not snap to raw frame-to-frame values")
    }
}
