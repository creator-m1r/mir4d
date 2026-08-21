import XCTest
import simd
import Combine
@testable import MirUIHandGesture

@MainActor
final class MIRHandTrackingSessionTests: XCTestCase {
    private func configuredSession() -> MIRHandTrackingSession {
        let session = MIRHandTrackingSession()
        var config = MIRHandGestureConfiguration()
        config.minimumIntentConfidence = 0.1
        session.configuration = config
        return session
    }

    func testSessionForwardsIntentsFromPipeline() {
        let session = configuredSession()
        var received: [MIRHandIntent] = []
        let cancellable = session.intentPublisher.sink { received.append($0) }

        let frames = (0..<5).map { _ in [MIRHandPoseMock.mockFist()] }
        session.processFramesForTesting(frames)

        cancellable.cancel()
        XCTAssertFalse(received.isEmpty, "Session must forward intents from the recognition pipeline to intentPublisher")
        XCTAssertTrue(
            received.contains { $0.gesture.type != .rest },
            "A recognised gesture (e.g. fist) should produce a non-rest intent"
        )
    }

    func testSessionEmitsPinchIntent() {
        let session = configuredSession()
        var config = MIRHandGestureConfiguration()
        config.minimumIntentConfidence = 0.1

        config.recognizer.classifying.pinchScale = 0.5
        session.configuration = config

        var sawPinch = false
        let cancellable = session.intentPublisher.sink { intent in
            if intent.gesture.type == .pinch { sawPinch = true }
        }

        let frames = (0..<5).map { _ in [MIRHandPoseMock.mockPinch()] }
        session.processFramesForTesting(frames)

        cancellable.cancel()
        XCTAssertTrue(sawPinch, "mockPinch must be classified as a pinch intent")
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
}
