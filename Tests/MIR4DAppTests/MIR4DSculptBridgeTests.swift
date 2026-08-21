import XCTest
@testable import MIR4DApp
@testable import MirUIHandGesture
import Combine
import os
import simd

@MainActor
final class MIR4DSculptBridgeTests: XCTestCase {

    func testHandAdapterEmitsSculptIntentForSculptTarget() {
        let box = OSAllocatedUnfairLock<MIR4DSculptIntent?>(initialState: nil)
        let exp = expectation(description: "sculpt emitted")
        let sub = MIR4DSculptIntentPublisher.shared.stream.sink { intent in
            box.withLock { $0 = intent }
            exp.fulfill()
        }

        let adapter = MIRSpatialMenuHandAdapter.shared
        adapter.setInteractionTarget(.sculpt)
        let gesture = MIRHandGesture(
            type: .pinch,
            confidence: 0.9,
            position: SIMD3(0.4, 0.3, -0.2),
            strength: 0.8
        )
        let intent = MIRHandIntent(
            gesture: gesture,
            phase: .began,
            position: SIMD3(0.4, 0.3, -0.2),
            direction: .zero,
            strength: 0.8,
            confidence: 0.9
        )
        adapter.handle(intent)
        adapter.setInteractionTarget(.empty)

        wait(for: [exp], timeout: 1)
        let received = box.withLock { $0 }
        XCTAssertEqual(received?.mode, .push, "pinch maps to push mode")
        XCTAssertEqual(received?.depth, -0.2, "real Vision depth must be carried")
        XCTAssertEqual(received?.strength, 0.8)
        sub.cancel()
    }
}
