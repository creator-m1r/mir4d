import XCTest
@testable import MIR4DApp
import simd

final class TwoHandGestureTests: XCTestCase {
    private var controller = MIRAirGestureController()

    func testScaleDetected() {
        _ = controller.ingest(left: SIMD3(-0.10, 0, 0), right: SIMD3(0.10, 0, 0),
                              leftPinch: 0, rightPinch: 0, leftGrab: false, rightGrab: false, timestamp: Date())
        let second = controller.ingest(left: SIMD3(-0.15, 0, 0), right: SIMD3(0.15, 0, 0),
                                       leftPinch: 0, rightPinch: 0, leftGrab: false, rightGrab: false, timestamp: Date())
        XCTAssertEqual(second?.type, .twoHandScale)
    }

    func testRotateDetected() {
        _ = controller.ingest(left: SIMD3(-0.10, -0.10, 0), right: SIMD3(0.10, 0.10, 0),
                              leftPinch: 0, rightPinch: 0, leftGrab: false, rightGrab: false, timestamp: Date())
        let second = controller.ingest(left: SIMD3(-0.10, 0.10, 0), right: SIMD3(0.10, -0.10, 0),
                                       leftPinch: 0, rightPinch: 0, leftGrab: false, rightGrab: false, timestamp: Date())
        XCTAssertEqual(second?.type, .twoHandRotate)
    }

    func testTranslateDetected() {
        _ = controller.ingest(left: SIMD3(-0.10, 0, 0), right: SIMD3(0.10, 0, 0),
                              leftPinch: 0, rightPinch: 0, leftGrab: false, rightGrab: false, timestamp: Date())
        let second = controller.ingest(left: SIMD3(-0.10, 0.05, 0), right: SIMD3(0.10, 0.05, 0),
                                       leftPinch: 0, rightPinch: 0, leftGrab: false, rightGrab: false, timestamp: Date())
        XCTAssertEqual(second?.type, .twoHandTranslate)
    }

    func testTwoHandPinchDetected() {
        let result = controller.ingest(left: SIMD3(-0.10, 0, 0), right: SIMD3(0.10, 0, 0),
                                       leftPinch: 0.9, rightPinch: 0.9, leftGrab: false, rightGrab: false, timestamp: Date())
        XCTAssertEqual(result?.type, .twoHandPinch)
    }

    func testTwoHandGrabDetected() {
        let result = controller.ingest(left: SIMD3(-0.10, 0, 0), right: SIMD3(0.10, 0, 0),
                                       leftPinch: 0, rightPinch: 0, leftGrab: true, rightGrab: true, timestamp: Date())
        XCTAssertEqual(result?.type, .twoHandGrab)
    }

    func testHandsTooCloseYieldsNoGesture() {
        let result = controller.ingest(left: SIMD3(-0.01, 0, 0), right: SIMD3(0.01, 0, 0),
                                       leftPinch: 0, rightPinch: 0, leftGrab: false, rightGrab: false, timestamp: Date())
        XCTAssertNil(result)
    }

    // MARK: - Session integration (camera-less)

    @MainActor
    func testSessionEmitsTwoHandScale() async {
        let session = MIRHandTrackingSession()
        session.startMock(MIRHandPoseMock.mockTwoHandScale())
        let expectation = expectation(description: "two-hand intent")
        Task {
            try? await Task.sleep(nanoseconds: 300_000_000)
            let found = session.lastIntents.contains { $0.gesture.type == .twoHandScale }
            if found { expectation.fulfill() }
        }
        await fulfillment(of: [expectation], timeout: 2)
        session.stop()
    }

    @MainActor
    func testSessionPairsLeftAndRight() async {
        let session = MIRHandTrackingSession()
        let frame = MIRHandPoseMock.mockTwoHandPair(
            leftCenter: SIMD3(0.4, 0.66, 0),
            rightCenter: SIMD3(0.6, 0.66, 0))
        session.startMock([frame])
        let expectation = expectation(description: "pair detected")
        Task {
            try? await Task.sleep(nanoseconds: 300_000_000)
            let pair = session.debugInfo?.spatialContext.twoHandGesture
            if pair != nil { expectation.fulfill() }
        }
        await fulfillment(of: [expectation], timeout: 2)
        session.stop()
    }
}
