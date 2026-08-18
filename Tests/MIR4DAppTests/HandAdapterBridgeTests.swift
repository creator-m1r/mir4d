import XCTest
@testable import MIR4DApp
@testable import MirUIHandGesture
import simd

/// Integration tests for the hand-gesture → Spatial Menu bridge.
///
/// They verify the contract from §33 of the spec: the hand module never
/// triggers operations directly, it emits `MIRHandIntent`, and the adapter
/// translates those intents into the *same* Spatial Menu event path the
/// trackpad and voice channels use.
final class HandAdapterBridgeTests: XCTestCase {
    private var beganObserver: NSObjectProtocol?
    private var movedObserver: NSObjectProtocol?
    private var endedObserver: NSObjectProtocol?

    @MainActor
    override func setUp() {
        super.setUp()
        MIRSpatialMenuHandAdapter.shared.stop()
        MIRSpatialMenuGesture.shared.stop()
        MIRHandGestureModule.shared.stop()
    }

    @MainActor
    override func tearDown() {
        MIRSpatialMenuHandAdapter.shared.stop()
        MIRSpatialMenuGesture.shared.stop()
        MIRHandGestureModule.shared.stop()
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

    // MARK: - Direct translation (no camera)

    @MainActor
    func testPinchBeganOpensSpatialMenu() {
        var began = false
        beganObserver = NotificationCenter.default.addObserver(forName: .mir4DSpatialMenuBegan, object: nil, queue: .main) { _ in
            began = true
        }
        MIRSpatialMenuHandAdapter.shared.start()
        MIRSpatialMenuHandAdapter.shared.handle(intent(.pinch, .began))
        XCTAssertTrue(began, "A pinch 'began' intent must open the Spatial Menu")
    }

    @MainActor
    func testPointMoveDrivesSpatialMenuMovement() {
        var moved = false
        var movedDX: Double = 0
        movedObserver = NotificationCenter.default.addObserver(forName: .mir4DSpatialMenuMoved, object: nil, queue: .main) { note in
            moved = true
            movedDX = (note.userInfo?["dx"] as? Double) ?? 0
        }
        MIRSpatialMenuHandAdapter.shared.start()
        MIRSpatialMenuHandAdapter.shared.handle(intent(.pinch, .began))
        MIRSpatialMenuHandAdapter.shared.handle(intent(.point, .changed, position: SIMD3(0.02, 0, 0)))
        XCTAssertTrue(moved, "A tracking 'point' intent must move the Spatial Menu")
        XCTAssertGreaterThan(movedDX, 0, "Movement must carry a positive delta")
    }

    @MainActor
    func testPinchEndedCommitsSpatialMenu() {
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

    // MARK: - End-to-end through the module (camera-less mock source)

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
        wait(for: [exp], timeout: 3)
        MIRHandGestureModule.shared.stop()
        XCTAssertTrue(began, "A pinch recognised by the module must open the Spatial Menu")
    }
}
