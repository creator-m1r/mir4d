import XCTest
@testable import MIR4DApp
import simd

final class MotionTests: XCTestCase {
    func testVelocityIsSmoothed() {
        var motion = MIRHandMotion()
        let start = SIMD3(0.0, 0.0, 0.0)
        let result1 = motion.update(position: start, timestamp: Date())
        XCTAssertEqual(result1.speed, 0, accuracy: 1e-6)

        let moved = SIMD3(0.1, 0.0, 0.0)
        let result2 = motion.update(position: moved, timestamp: Date().addingTimeInterval(0.1))
        XCTAssertGreaterThan(result2.speed, 0)
        XCTAssertLessThan(result2.speed, 2.0, "Raw speed would be ~1.0; smoothing keeps it bounded")
    }

    func testSingleFrameHasNoVelocity() {
        var motion = MIRHandMotion()
        let r = motion.update(position: SIMD3(0.5, 0.5, 0.5), timestamp: Date())
        XCTAssertEqual(r.velocity, .zero)
        XCTAssertEqual(r.acceleration, .zero)
    }

    func testDirectionFollowsMovement() {
        var motion = MIRHandMotion()
        _ = motion.update(position: SIMD3(0, 0, 0), timestamp: Date())
        let r = motion.update(position: SIMD3(1, 0, 0), timestamp: Date().addingTimeInterval(0.1))
        XCTAssertGreaterThan(r.direction.x, 0.9)
    }

    func testHandLossResetsMotion() {
        var recognizer = MIRHandGestureRecognizer()
        let mapper = MIRHandSpatialMapper()
        let pose = MIRHandPoseMock.mockPoint()
        _ = recognizer.ingest(pose: pose, scenePosition: mapper.map(normalized: pose.palmPosition), timestamp: Date())
        let lost = recognizer.handleMissing(timestamp: Date().addingTimeInterval(1))
        XCTAssertNotNil(lost)
        XCTAssertEqual(recognizer.currentState, .lost)
    }
}
