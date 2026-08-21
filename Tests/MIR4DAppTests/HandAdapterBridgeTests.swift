import XCTest
@testable import MIR4DApp
@testable import MirUIHandGesture
import simd

final class HandAdapterBridgeTests: XCTestCase {
    private var beganObserver: NSObjectProtocol?
    private var movedObserver: NSObjectProtocol?
    private var endedObserver: NSObjectProtocol?

    override func setUp() {
        super.setUp()
        MainActor.assumeIsolated {
            MIRSpatialMenuHandAdapter.shared.stop()
            MIRSpatialMenuGesture.shared.stop()
            MIRHandGestureModule.shared.stop()
        }
    }

    override func tearDown() {
        MainActor.assumeIsolated {
            MIRSpatialMenuHandAdapter.shared.stop()
            MIRSpatialMenuGesture.shared.stop()
            MIRHandGestureModule.shared.stop()
        }
        if let o = beganObserver { NotificationCenter.default.removeObserver(o) }
        if let o = movedObserver { NotificationCenter.default.removeObserver(o) }
        if let o = endedObserver { NotificationCenter.default.removeObserver(o) }
        super.tearDown()
    }

    private func intent(_ type: MIRHandGestureType, _ phase: MIRHandIntentPhase,
                        position: SIMD3<Double> = .zero, strength: Double = 0.5) -> MIRHandIntent {
        MIRHandIntent(
            gesture: MIRHandGesture(type: type, confidence: 0.9, position: position, strength: strength),
            phase: phase,
            position: position,
            direction: .zero,
            strength: strength,
            confidence: 0.9
        )
    }

    @MainActor
    func testPinchBeganOpensSpatialMenu() async {
        var began = false
        beganObserver = NotificationCenter.default.addObserver(forName: .mir4DSpatialMenuBegan, object: nil, queue: .main) { _ in
            began = true
        }
        MIRSpatialMenuHandAdapter.shared.start()
        MIRSpatialMenuHandAdapter.shared.handle(intent(.pinch, .began))
        XCTAssertTrue(began, "A pinch 'began' intent must open the Spatial Menu")
    }

    @MainActor
    func testPointMoveDrivesSpatialMenuMovement() async {
        var movedDX: Double = 0
        let exp = expectation(description: "spatial menu moved")
        movedObserver = NotificationCenter.default.addObserver(forName: .mir4DSpatialMenuMoved, object: nil, queue: .main) { note in
            movedDX = (note.userInfo?["dx"] as? Double) ?? 0
            exp.fulfill()
        }
        MIRSpatialMenuHandAdapter.shared.start()
        MIRSpatialMenuHandAdapter.shared.handle(intent(.pinch, .began))
        MIRSpatialMenuHandAdapter.shared.handle(intent(.point, .changed, position: SIMD3(0.02, 0, 0)))
        await fulfillment(of: [exp], timeout: 2)
        XCTAssertGreaterThan(movedDX, 0, "A tracking 'point' intent must move the Spatial Menu with a positive delta")
    }

    @MainActor
    func testPinchEndedCommitsSpatialMenu() async {
        var ended = false
        var commit = false
        endedObserver = NotificationCenter.default.addObserver(forName: .mir4DSpatialMenuEnded, object: nil, queue: .main) { note in
            ended = true
            commit = (note.userInfo?["commit"] as? Bool) ?? false
        }
        MIRSpatialMenuHandAdapter.shared.start()
        MIRSpatialMenuHandAdapter.shared.handle(intent(.pinch, .began))
        MIRSpatialMenuHandAdapter.shared.handle(intent(.pinch, .ended))
        XCTAssertTrue(ended, "A pinch 'ended' intent must close the Spatial Menu")
        XCTAssertTrue(commit, "Releasing the pinch must commit the selection")
    }

    @MainActor
    func testModuleMockPinchOpensSpatialMenu() async {
        var began = false
        beganObserver = NotificationCenter.default.addObserver(forName: .mir4DSpatialMenuBegan, object: nil, queue: .main) { _ in
            began = true
        }
        MIRSpatialMenuHandAdapter.shared.start()
        let frames = Array(repeating: [MIRHandPoseMock.mockPinch()], count: 5)
        MIRHandGestureModule.shared.startMock(frames, mode: .once)
        let exp = expectation(description: "spatial menu opened by hand pinch")
        Task {
            try? await Task.sleep(nanoseconds: 500_000_000)
            exp.fulfill()
        }
        await fulfillment(of: [exp], timeout: 3)
        MIRHandGestureModule.shared.stop()
        XCTAssertTrue(began, "A pinch recognised by the module must open the Spatial Menu")
    }
}
