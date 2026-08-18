import XCTest
import CoreGraphics
@testable import MIR4DApp

final class MIR4DSketchIntentTests: XCTestCase {
    func testSketchIntentCarriesPointsAndPhase() {
        let points = [CGPoint(x: -0.5, y: -0.5), CGPoint(x: 0, y: 0), CGPoint(x: 0.5, y: 0.5)]
        let live = MIR4DSketchIntent(points: points, live: true)
        let committed = MIR4DSketchIntent(points: points, live: false)

        XCTAssertEqual(live.points.count, 3)
        XCTAssertTrue(live.live)
        XCTAssertFalse(committed.live)
    }

    func testDegenerateStrokeHasTooFewPoints() {
        let intent = MIR4DSketchIntent(points: [CGPoint(x: 0, y: 0)], live: false)
        XCTAssertEqual(intent.points.count, 1)
    }
}
