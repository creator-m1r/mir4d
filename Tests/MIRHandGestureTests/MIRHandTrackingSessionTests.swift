import XCTest
import simd
import Combine
@testable import MirUIHandGesture

/// Exercises the real `MIRHandTrackingSession` pipeline (source → mapper →
/// recognizers → intents) via the camera-less mock source. This is the same
/// code path the camera uses, minus the Vision `VNHandPoseRequest` detection.
@MainActor
final class MIRHandTrackingSessionTests: XCTestCase {
    func testSessionForwardsIntentsFromSource() async {
        let session = MIRHandTrackingSession()
        var config = MIRHandGestureConfiguration()
        config.minimumIntentConfidence = 0.1
        session.configuration = config

        let exp = expectation(description: "intent emitted from session")
        var received: [MIRHandIntent] = []
        let cancellable = session.intentPublisher.sink { intent in
            received.append(intent)
            exp.fulfill()
        }

        let frames = (0..<5).map { _ in [MIRHandPoseMock.mockFist()] }
        session.startMock(frames)

        await fulfillment(of: [exp], timeout: 2)

        XCTAssertFalse(received.isEmpty, "Session must forward intents from the tracking source to intentPublisher")
        XCTAssertTrue(
            received.contains { $0.gesture.type != .rest },
            "A recognised gesture (e.g. fist) should produce a non-rest intent"
        )

        cancellable.cancel()
        session.stop()
    }

    func testSessionStatusTransitionsToRunning() {
        let session = MIRHandTrackingSession()
        var config = MIRHandGestureConfiguration()
        config.minimumIntentConfidence = 0.1
        session.configuration = config

        session.startMock([[MIRHandPoseMock.mockOpenPalm()]])
        XCTAssertEqual(session.status, .running, "Session must report running once a source is active")

        session.stop()
        XCTAssertEqual(session.status, .inactive, "Session must report inactive after stop")
    }

    func testSessionEmitsPinchIntent() async {
        let session = MIRHandTrackingSession()
        var config = MIRHandGestureConfiguration()
        config.minimumIntentConfidence = 0.1
        // Lower the pinch threshold so the synthetic mockPinch (pinch ~0.9) is classified.
        config.recognizer.classifying.pinchScale = 0.5
        session.configuration = config

        let exp = expectation(description: "pinch intent emitted")
        var sawPinch = false
        let cancellable = session.intentPublisher.sink { intent in
            if intent.gesture.type == .pinch { sawPinch = true; exp.fulfill() }
        }

        let frames = (0..<5).map { _ in [MIRHandPoseMock.mockPinch()] }
        session.startMock(frames)

        await fulfillment(of: [exp], timeout: 2)
        XCTAssertTrue(sawPinch, "mockPinch must be classified as a pinch intent")

        cancellable.cancel()
        session.stop()
    }
}
