import Foundation
import simd
import Combine
import MirUIHandGesture

/// Maps normalised hand-scene coordinates into CAD world space.
///
/// v0.1 default is an identity mapping: the spatial-mapper interaction volume
/// is already centred on the scene origin / camera target, so the hand point
/// lands inside the visible frustum. `scale`/`offset` are exposed for a later
/// calibration UI (per-user reach, desk height, camera framing).
struct MIRHandCalibration: Sendable {
    var scale: SIMD3<Double> = .init(1, 1, 1)
    var offset: SIMD3<Double> = .zero

    func map(_ hand: SIMD3<Double>) -> SIMD3<Double> {
        hand * scale + offset
    }
}

/// Temporal filtering for the hand grab: pinch dead-zone/hysteresis plus
/// position smoothing and velocity estimation. Keeps the interaction stable
/// against raw recognition buzz without adding latency.
struct MIRHandTemporalFilter: Sendable {
    /// Pinch strength (0..1) that arms the grab.
    var pinchBegin: Double = 0.65
    /// Pinch strength (0..1) that releases the grab (hysteresis vs `pinchBegin`).
    var pinchEnd: Double = 0.40
    /// Exponential smoothing factor for the hand position (0..1; lower = smoother).
    /// 0.6 damps raw recognition jitter without adding noticeable drag latency.
    var smoothing: Double = 0.6

    private var active = false
    private var smoothedPosition: SIMD3<Double>?
    private var lastPosition: SIMD3<Double>?
    private var lastTime: Date?

    /// Hysteresis gate for the pinch strength. Returns the stable on/off state.
    mutating func updatePinch(_ strength: Double) -> Bool {
        if active {
            if strength < pinchEnd { active = false }
        } else if strength > pinchBegin {
            active = true
        }
        return active
    }

    /// Smooths the hand position and estimates instantaneous velocity (world/s).
    mutating func updatePosition(_ position: SIMD3<Double>, at time: Date = Date()) -> (position: SIMD3<Double>, velocity: SIMD3<Double>) {
        let a = min(max(smoothing, 0), 1)
        let newPos: SIMD3<Double>
        if let prev = smoothedPosition {
            newPos = prev * (1 - a) + position * a
        } else {
            newPos = position
        }
        smoothedPosition = newPos

        var velocity = SIMD3<Double>.zero
        if let last = lastPosition, let lt = lastTime {
            let dt = time.timeIntervalSince(lt)
            if dt > 1e-4 {
                velocity = (newPos - last) / dt
            }
        }
        lastPosition = newPos
        lastTime = time
        return (newPos, velocity)
    }

    mutating func reset() {
        active = false
        smoothedPosition = nil
        lastPosition = nil
        lastTime = nil
    }
}

/// Closed-loop hand grab controller for Vertical Slice v0.1.
///
/// Pipeline: `Pinch → point → ray (camera → hand) → pick → grab → move → commit`.
/// Translation-only in v0.1 (rotation/scale arrive with two-hand in v0.2).
/// The controller lives in the App target so it can drive the real MirEngine
/// through `MIR4DModelRuntime`; it owns interaction state only.
@MainActor
final class MIRHandGrabController {
    enum State: Equatable {
        case idle
        case grabbing(objectId: UInt64)
    }

    private(set) var state: State = .idle
    private var filter = MIRHandTemporalFilter()
    private var calibration = MIRHandCalibration()
    private var cancellables = Set<AnyCancellable>()
    private var tickTask: Task<Void, Never>?

    /// Grab anchor: object seed transform + the hand world point at grab start,
    /// so preview keeps the grabbed point under the hand (relative offset).
    private var seedTransform: MirTransform?
    private var grabAnchorWorld: SIMD3<Double> = .zero
    private var lastIntentTime: Date = .distantPast

    /// Lost-tracking grace period: hold the preview, then commit so the object
    /// stays where it was last moved instead of snapping back. Kept generous
    /// (1.0s) because camera hand-tracking routinely drops a few frames; a short
    /// grace made the grab "let go" mid-drag whenever recognition blinked.
    var graceInterval: TimeInterval = 1.0

    func start() {
        guard cancellables.isEmpty else { return }
        MIRHandGestureModule.shared.session.intentPublisher
            .sink { [weak self] intent in
                Task { @MainActor in self?.handle(intent: intent) }
            }
            .store(in: &cancellables)

        tickTask = Task { [weak self] in
            while !Task.isCancelled {
                try? await Task.sleep(nanoseconds: 100_000_000)
                self?.tick()
            }
        }
    }

    func stop() {
        tickTask?.cancel()
        tickTask = nil
        cancellables.removeAll()
        if case .grabbing(let id) = state {
            MIR4DModelRuntime.shared.cancelGrab()
            _ = MIR4DModelRuntime.shared.commitGrab(objectId: id)
        }
        filter.reset()
        state = .idle
    }

    // MARK: - Intent handling

    private func handle(intent: MIRHandIntent) {
        // Only pinch drives the grab in v0.1.
        guard intent.gesture.type == .pinch else {
            releaseIfGrabbing(reason: "non-pinch gesture")
            return
        }

        let pinchActive = filter.updatePinch(intent.strength)
        let (world, _) = filter.updatePosition(calibration.map(intent.position), at: intent.timestamp)
        lastIntentTime = intent.timestamp

        if !pinchActive {
            releaseIfGrabbing(reason: "pinch released")
            updateHover(world: world)
            return
        }

        switch state {
        case .idle:
            beginGrab(world: world)
        case .grabbing(let id):
            previewGrab(objectId: id, world: world)
        }
    }

    private func tick() {
        if case .grabbing = state,
           Date().timeIntervalSince(lastIntentTime) > graceInterval {
            // Tracking lost: finalize the move so the object stays in place.
            releaseIfGrabbing(reason: "tracking lost (grace period)")
        }
    }

    // MARK: - Grab phases

    private func beginGrab(world: SIMD3<Double>) {
        guard let eye = MIR4DModelRuntime.shared.cameraEye() else { return }
        let dir = world - eye
        guard simd_length(dir) > 1e-6 else { return }
        guard let hit = MIR4DModelRuntime.shared.pickHandRay(origin: eye, direction: simd_normalize(dir)) else {
            return
        }
        guard let seed = MIR4DModelRuntime.shared.getObjectTransform(objectId: hit.objectId) else { return }

        seedTransform = seed
        grabAnchorWorld = world
        MIR4DModelRuntime.shared.beginGrab(objectId: hit.objectId)
        state = .grabbing(objectId: hit.objectId)
    }

    private func previewGrab(objectId: UInt64, world: SIMD3<Double>) {
        guard let seed = seedTransform else { return }
        // Keep the grabbed point under the hand: offset = seedPos - anchorWorld.
        let offset = SIMD3(seed.px, seed.py, seed.pz) - grabAnchorWorld
        let newPos = world + offset
        var t = seed
        t.px = newPos.x
        t.py = newPos.y
        t.pz = newPos.z
        MIR4DModelRuntime.shared.previewGrab(objectId: objectId, transform: t)
    }

    private func releaseIfGrabbing(reason: String) {
        guard case .grabbing(let id) = state else { return }
        _ = MIR4DModelRuntime.shared.commitGrab(objectId: id)
        seedTransform = nil
        state = .idle
        filter.reset()
        MIR4DLog("HAND", "grab committed (\(reason))")
    }

    private func updateHover(world: SIMD3<Double>) {
        guard let eye = MIR4DModelRuntime.shared.cameraEye() else { return }
        let dir = world - eye
        guard simd_length(dir) > 1e-6 else { return }
        if let hit = MIR4DModelRuntime.shared.pickHandRay(origin: eye, direction: simd_normalize(dir)) {
            MIR4DModelRuntime.shared.setHandHover(objectId: hit.objectId)
        }
    }
}
