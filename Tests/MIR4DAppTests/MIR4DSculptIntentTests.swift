import XCTest
@testable import MIR4DApp
@testable import MirUIHandGesture
import CoreGraphics
import simd

/// Unit tests for the typed sculpt intent payload.
final class MIR4DSculptIntentTests: XCTestCase {

    private func handIntent(gesture: MIRHandGestureType, strength: CGFloat, position: SIMD3<Double> = SIMD3(10, 20, 0)) -> MIRHandIntent {
        MIRHandIntent(
            gesture: MIRHandGesture(type: gesture, confidence: 0.9, position: position, strength: strength),
            phase: .changed,
            position: position,
            direction: .zero,
            strength: strength,
            confidence: 0.9
        )
    }

    func testDefaults() {
        let s = MIR4DSculptIntent(position: CGPoint(x: 1, y: 2))
        XCTAssertEqual(s.position, CGPoint(x: 1, y: 2))
        XCTAssertEqual(s.depth, 0)
        XCTAssertEqual(s.direction, .zero)
        XCTAssertEqual(s.pressure, 0)
        XCTAssertEqual(s.radius, 1)
        XCTAssertEqual(s.strength, 0)
        XCTAssertEqual(s.velocity, 0)
        XCTAssertEqual(s.mode, .push)
    }

    func testInitFromGrabSelectsGrabMode() {
        let s = MIR4DSculptIntent(from: handIntent(gesture: .grab, strength: 0.7))
        XCTAssertEqual(s.mode, .grab)
        XCTAssertEqual(s.position, CGPoint(x: 10, y: 20))
        XCTAssertEqual(s.pressure, 0.7)
        XCTAssertEqual(s.strength, 0.7)
    }

    func testInitFromCarriesDepth() {
        let s = MIR4DSculptIntent(from: handIntent(gesture: .pinch, strength: 0.5, position: SIMD3(10, 20, -0.4)))
        XCTAssertEqual(s.depth, -0.4)
        XCTAssertEqual(s.position, CGPoint(x: 10, y: 20))
    }

    func testInitFromPinchSelectsPushMode() {
        let s = MIR4DSculptIntent(from: handIntent(gesture: .pinch, strength: 0.5))
        XCTAssertEqual(s.mode, .push)
    }

    func testStrengthIsClamped() {
        let s = MIR4DSculptIntent(from: handIntent(gesture: .pinch, strength: 5))
        XCTAssertEqual(s.pressure, 1)
        XCTAssertEqual(s.strength, 1)
    }

    func testEquatable() {
        let a = MIR4DSculptIntent(position: .zero, mode: .smooth)
        let b = MIR4DSculptIntent(position: .zero, mode: .smooth)
        let c = MIR4DSculptIntent(position: .zero, mode: .cut)
        XCTAssertEqual(a, b)
        XCTAssertNotEqual(a, c)
    }
}
