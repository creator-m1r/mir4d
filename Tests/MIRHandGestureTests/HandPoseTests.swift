import XCTest
@testable import MIR4DApp

final class HandPoseTests: XCTestCase {
    func testMockPoseHasFullSkeleton() {
        let pose = MIRHandPoseMock.mockOpenPalm()
        XCTAssertEqual(pose.landmarks.count, 21)
        XCTAssertEqual(Set(pose.landmarks.map(\.id)).count, 21, "All landmark ids must be unique")
    }

    func testHandednessIsPreserved() {
        let left = MIRHandPoseMock.mockPoint(handedness: .left)
        let right = MIRHandPoseMock.mockPoint(handedness: .right)
        XCTAssertEqual(left.handedness, .left)
        XCTAssertEqual(right.handedness, .right)
    }

    func testLandmarkLookup() {
        let pose = MIRHandPoseMock.mockPinch()
        XCTAssertNotNil(pose.landmark(.wrist))
        XCTAssertNotNil(pose.landmark(.thumbTip))
        XCTAssertNil(pose.landmark(.wrist).flatMap { _ in nil })
    }

    func testConfidenceIsBounded() {
        let pose = MIRHandPoseMock.mockGrab()
        XCTAssertGreaterThanOrEqual(pose.confidence, 0)
        XCTAssertLessThanOrEqual(pose.confidence, 1)
    }
}
