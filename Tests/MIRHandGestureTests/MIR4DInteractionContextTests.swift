import XCTest
@testable import MirUIHandGesture
import simd

/// Unit tests for the scene-aware gesture → semantic-action resolver.
final class MIR4DInteractionContextTests: XCTestCase {

    private func resolve(_ target: MIR4DInteractionTarget, _ gesture: MIRHandGestureType,
                         _ phase: MIRHandIntentPhase) -> MIR4DInteractionAction {
        MIR4DInteractionContext(target: target).resolve(gesture: gesture, phase: phase)
    }

    // MARK: - Canonical mappings from the hand-gesture architecture

    func testEmptyPinchIsCameraControl() {
        XCTAssertEqual(resolve(.empty, .pinch, .began), .cameraControl)
    }

    func testEmptyPointOpensMenu() {
        XCTAssertEqual(resolve(.empty, .point, .changed), .openMenu)
    }

    func testObjectPinchMovesObject() {
        XCTAssertEqual(resolve(.object, .pinch, .began), .moveObject)
        XCTAssertEqual(resolve(.object, .pinch, .changed), .moveObject)
    }

    func testFacePinchSelectsFace() {
        XCTAssertEqual(resolve(.face, .pinch, .began), .selectFace)
    }

    func testEdgePinchSelectsEdge() {
        XCTAssertEqual(resolve(.edge, .pinch, .began), .selectEdge)
    }

    func testSculptPinchSculpts() {
        XCTAssertEqual(resolve(.sculpt, .pinch, .began), .sculpt)
        XCTAssertEqual(resolve(.sculpt, .grab, .changed), .sculpt)
    }

    func testSketchPinchPaints() {
        XCTAssertEqual(resolve(.sketch, .pinch, .began), .paint)
    }

    func testParameterPinchEditsParameter() {
        XCTAssertEqual(resolve(.parameter, .pinch, .began), .editParameter)
    }

    func testNavigationPinchNavigates() {
        XCTAssertEqual(resolve(.navigation, .pinch, .began), .navigate)
    }

    // MARK: - Release always confirms

    func testPinchReleaseConfirms() {
        XCTAssertEqual(resolve(.object, .pinch, .ended), .confirm)
        XCTAssertEqual(resolve(.face, .pinch, .cancelled), .confirm)
        XCTAssertEqual(resolve(.sculpt, .pinch, .ended), .confirm)
    }

    // MARK: - Two-hand relational gestures

    func testTwoHandGesturesNavigate() {
        XCTAssertEqual(resolve(.empty, .twoHandScale, .changed), .navigate)
        XCTAssertEqual(resolve(.object, .twoHandRotate, .began), .navigate)
        XCTAssertEqual(resolve(.face, .twoHandTranslate, .changed), .navigate)
    }

    // MARK: - Unknown combinations fall back to opening the menu

    func testUnknownCombinationFallsBackToMenu() {
        XCTAssertEqual(resolve(.window, .grab, .began), .openMenu)
        XCTAssertEqual(resolve(.timeline, .fist, .began), .openMenu)
    }
}
