import XCTest
@testable import MirUIHandGesture
import simd

final class HandIntentTests: XCTestCase {
    private func sampleIntent(phase: MIRHandIntentPhase) -> MIRHandIntent {
        let gesture = MIRHandGesture(
            type: .pinch,
            confidence: 0.91,
            position: SIMD3(0.1, 0.2, 0.3),
            direction: SIMD3(1, 0, 0),
            velocity: SIMD3(0.5, 0, 0),
            strength: 0.7
        )
        return MIRHandIntent(
            gesture: gesture,
            phase: phase,
            position: gesture.position,
            direction: gesture.direction,
            strength: gesture.strength,
            confidence: gesture.confidence
        )
    }

    func testIntentCarriesGestureContext() {
        let intent = sampleIntent(phase: .began)
        XCTAssertEqual(intent.gesture.type, .pinch)
        XCTAssertEqual(intent.phase, .began)
        XCTAssertGreaterThan(intent.confidence, 0)
        XCTAssertGreaterThan(intent.strength, 0)
    }

    func testIntentConvertsToMIRIntent() {
        let intent = sampleIntent(phase: .changed)
        let mir = intent.toMIRIntent()
        XCTAssertEqual(mir.source, .spatial)
        XCTAssertEqual(mir.action, "PINCH")
        XCTAssertEqual(mir.phase, .preview)
    }

    func testIntentPhasesMapCorrectly() {
        XCTAssertEqual(sampleIntent(phase: .began).toMIRIntent().phase, .selection)
        XCTAssertEqual(sampleIntent(phase: .changed).toMIRIntent().phase, .preview)
        XCTAssertEqual(sampleIntent(phase: .ended).toMIRIntent().phase, .confirmation)
        XCTAssertEqual(sampleIntent(phase: .cancelled).toMIRIntent().phase, .cancel)
    }

    @MainActor
    func testIntentPublishesToRouter() async {
        let intent = sampleIntent(phase: .began)
        MIRIntentRouter.shared.publish(intent.toMIRIntent())
        XCTAssertEqual(MIRIntentRouter.shared.latestIntent?.action, "PINCH")
    }

    func testContactBridgesToAirField() {
        let contact = MIRHandContact(
            position: SIMD3(1, 2, 3),
            velocity: SIMD3(0.1, 0, 0),
            radius: 0.05,
            strength: 0.6,
            state: .pressing
        )
        let field = contact.toAirContactField()
        XCTAssertEqual(field.state, .pressing)
        XCTAssertEqual(field.strength, 0.6)
        XCTAssertTrue(field.isActive)
    }

    func testConfidenceIsClamped() {
        let gesture = MIRHandGesture(type: .point, confidence: 5.0, position: .zero, strength: -1)
        XCTAssertLessThanOrEqual(gesture.confidence, 1)
        XCTAssertGreaterThanOrEqual(gesture.strength, 0)
    }
}
