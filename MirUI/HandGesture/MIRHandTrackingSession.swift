import Foundation
import simd
import Combine

/// Status of the hand-tracking pipeline.
enum MIRHandTrackingStatus: Sendable {
    case inactive
    case running
    case cameraUnavailable
}

/// Orchestrates the full hand-gesture pipeline:
///
/// ```
/// Source → poses → mapper → per-hand recognizers → two-hand recognizer → intents
/// ```
///
/// Heavy processing (Vision, classification, motion) runs off the main actor.
/// Only intent emission and observable state touches the main actor, so the UI
/// thread is never blocked by recognition.
@MainActor
public final class MIRHandTrackingSession: ObservableObject {
    @Published var status: MIRHandTrackingStatus = .inactive
    @Published private(set) var lastIntents: [MIRHandIntent] = []
    @Published private(set) var debugInfo: MIRHandGestureDebugInfo?

    private let intentSubject = PassthroughSubject<MIRHandIntent, Never>()
    /// Stream of recognised hand intents for downstream consumers (Spatial Menu, CAD).
    public var intentPublisher: AnyPublisher<MIRHandIntent, Never> {
        intentSubject.eraseToAnyPublisher()
    }

    var configuration: MIRHandGestureConfiguration {
        didSet { poolConfiguration = configuration }
    }

    private var source: (any MIRHandTrackingSource)?
    private var task: Task<Void, Never>?
    private var poolConfiguration: MIRHandGestureConfiguration
    private let emitter = MIRHandIntentEmitter()

    init(configuration: MIRHandGestureConfiguration = .init()) {
        self.configuration = configuration
        self.poolConfiguration = configuration
    }

    /// Start tracking with a source. Defaults to the built-in camera source.
    /// If the camera is unavailable the session becomes `cameraUnavailable`
    /// instead of crashing.
    func start(with source: (any MIRHandTrackingSource)? = nil) {
        guard status != .running else { return }
        let src = source ?? MIRCameraTrackingSource()
        guard src.availability != .unavailable else {
            status = .cameraUnavailable
            return
        }
        self.source = src
        status = .running
        let cfg = poolConfiguration
        // Recognition must run off the main actor: the class is @MainActor, so a
        // plain `Task { }` would inherit it and block the UI every frame.
        task = Task.detached { [weak self] in
            let stream = src.start()
            let pool = MIRHandRecognizerPool(configuration: cfg)
            for await frame in stream {
                guard !Task.isCancelled else { break }
                let result = pool.process(frames: frame)
                await self?.publish(result)
            }
        }
    }

    /// Convenience test entry point: run a synthetic source without a camera.
    func startMock(_ frames: [[MIRHandPose]], mode: MIRMockTrackingSource.Mode = .once) {
        start(with: MIRMockTrackingSource(frames: frames, mode: mode))
    }

    func stop() {
        task?.cancel()
        task = nil
        source?.stop()
        source = nil
        status = .inactive
        lastIntents = []
        debugInfo = nil
    }

    /// Test seam: run pre-recorded frame batches through the exact recognition
    /// pipeline `start()` uses (recognizers → intents → `intentPublisher`) without
    /// spawning a tracking-source `Task`. XCTest's async-task observation crashes
    /// on long-lived unstructured tasks, so tests drive the pipeline synchronously.
    func processFramesForTesting(_ frames: [[MIRHandPose]]) {
        let pool = MIRHandRecognizerPool(configuration: poolConfiguration)
        for frame in frames {
            publish(pool.process(frames: frame))
        }
    }

    /// Current one- or two-hand state for the Spatial Menu to read.
    func spatialContext() -> MIRHandSpatialContext {
        debugInfo?.spatialContext ?? .empty
    }

    private func publish(_ result: MIRHandProcessingResult) {
        var emitted: [MIRHandIntent] = []
        for intent in result.intents where intent.confidence >= poolConfiguration.minimumIntentConfidence {
            emitter.publish(intent)
            intentSubject.send(intent)
            emitted.append(intent)
        }
        lastIntents = emitted
        if poolConfiguration.enableDebugOverlay {
            debugInfo = result.debug
        } else {
            debugInfo = MIRHandGestureDebugInfo(spatialContext: result.debug.spatialContext)
        }
    }
}

// MARK: - Background processing pool

/// Owns all recognition state. Runs on the background stream task; it is never
/// shared across actors, so it does not need to be `Sendable`.
private final class MIRHandRecognizerPool: @unchecked Sendable {
    var configuration: MIRHandGestureConfiguration
    var recognizers: [Handedness: MIRHandGestureRecognizer] = [:]
    var twoHandController = MIRAirGestureController()
    var twoHandCurrent: MIRHandGestureType = .rest
    var lastTwoHandStrength: Double = 0
    var lastTwoHandConfidence: Double = 0
    let mapper: MIRHandSpatialMapper

    init(configuration: MIRHandGestureConfiguration) {
        self.configuration = configuration
        self.mapper = configuration.mapper
        self.twoHandController.configuration = .init(
            minimumHandDistance: configuration.twoHand.minimumHandDistance,
            scaleDeadZone: configuration.twoHand.scaleDeadZone,
            rotationDeadZone: configuration.twoHand.rotationDeadZone,
            translationDeadZone: configuration.twoHand.translationDeadZone
        )
    }

    func process(frames: [MIRHandPose]) -> MIRHandProcessingResult {
        configuration.mapper = mapper
        var byHand: [Handedness: MIRHandPose] = [:]
        for p in frames {
            let key: Handedness = p.handedness == .unknown
                ? (byHand[.left] == nil ? .left : .right)
                : p.handedness
            if byHand[key] == nil { byHand[key] = p }
        }

        var intents: [MIRHandIntent] = []
        var handEntries: [MIRHandGestureDebugInfo.HandEntry] = []
        let now = Date()

        for hand in [Handedness.left, .right] {
            guard let pose = byHand[hand] else {
                if var rec = recognizers[hand] {
                    if let ev = rec.handleMissing(timestamp: now),
                       ev.gesture.confidence >= configuration.minimumIntentConfidence {
                        intents.append(makeIntent(ev, strength: ev.gesture.strength))
                    }
                    recognizers[hand] = rec
                }
                continue
            }

            let scenePos = mapper.map(normalized: pose.palmPosition)
            var rec = recognizers[hand] ?? MIRHandGestureRecognizer()
            rec.configuration = configuration.recognizer
            if let ev = rec.ingest(pose: pose, scenePosition: scenePos, timestamp: pose.timestamp),
               ev.gesture.confidence >= configuration.minimumIntentConfidence {
                intents.append(makeIntent(ev, strength: ev.gesture.strength))
            }
            recognizers[hand] = rec

            let classifier = MIRHandGestureClassifier()
            handEntries.append(.init(
                handedness: hand,
                gesture: rec.activeGesture,
                confidence: rec.currentState == .lost ? 0 : 1,
                pinch: classifier.pinchStrength(in: pose),
                speed: simd_length(rec.currentVelocity),
                direction: rec.currentDirection,
                position: scenePos,
                landmarkPositions: pose.landmarks.map { $0.normalizedPosition }
            ))
        }

        var twoHandEntry: MIRHandGestureDebugInfo.TwoHandEntry?
        if configuration.twoHandEnabled,
           let left = byHand[.left], let right = byHand[.right] {
            let lScene = mapper.map(normalized: left.palmPosition)
            let rScene = mapper.map(normalized: right.palmPosition)
            var cl = MIRHandGestureClassifier(); cl.configuration = configuration.recognizer.classifying
            let lRes = cl.classify(left)
            let rRes = cl.classify(right)
            let folded = configuration.recognizer.classifying.curlFolded
            let leftGrab = lRes.curls.values.filter { $0 >= folded }.count >= 4
            let rightGrab = rRes.curls.values.filter { $0 >= folded }.count >= 4

            if let res = twoHandController.ingest(
                left: lScene, right: rScene,
                leftPinch: lRes.pinchStrength, rightPinch: rRes.pinchStrength,
                leftGrab: leftGrab, rightGrab: rightGrab, timestamp: now
            ) {
                if res.type != twoHandCurrent {
                    if twoHandCurrent != .rest {
                        intents.append(makeTwoHandIntent(type: twoHandCurrent, phase: .ended,
                                                         strength: lastTwoHandStrength, confidence: lastTwoHandConfidence,
                                                         center: (lScene + rScene) * 0.5))
                    }
                    twoHandCurrent = res.type
                    intents.append(makeTwoHandIntent(type: res.type, phase: .began,
                                                     strength: res.strength, confidence: res.confidence,
                                                     center: (lScene + rScene) * 0.5))
                } else {
                    intents.append(makeTwoHandIntent(type: res.type, phase: .changed,
                                                     strength: res.strength, confidence: res.confidence,
                                                     center: (lScene + rScene) * 0.5))
                }
                lastTwoHandStrength = res.strength
                lastTwoHandConfidence = res.confidence
            } else if twoHandCurrent != .rest {
                intents.append(makeTwoHandIntent(type: twoHandCurrent, phase: .ended,
                                                 strength: lastTwoHandStrength, confidence: lastTwoHandConfidence,
                                                 center: (lScene + rScene) * 0.5))
                twoHandCurrent = .rest
            }

            twoHandEntry = .init(
                center: (lScene + rScene) * 0.5,
                distance: simd_distance(lScene, rScene),
                gesture: twoHandCurrent
            )
        }

        let context = MIRHandSpatialContext(
            hands: handEntries.map { MIRHandSpatialContext.HandState(handedness: $0.handedness, gesture: $0.gesture, position: $0.position, pinch: $0.pinch, direction: $0.direction) },
            twoHandGesture: twoHandEntry?.gesture ?? .rest
        )

        let debug = MIRHandGestureDebugInfo(
            hands: handEntries,
            twoHand: twoHandEntry,
            spatialContext: context
        )
        return MIRHandProcessingResult(intents: intents, debug: debug)
    }

    private func makeIntent(_ ev: MIRHandGestureEvent, strength: Double) -> MIRHandIntent {
        MIRHandIntent(
            gesture: ev.gesture,
            phase: MIRHandIntentPhase(rawValue: ev.phase.rawValue) ?? .changed,
            position: ev.gesture.position,
            direction: ev.gesture.direction,
            strength: strength,
            confidence: ev.gesture.confidence,
            timestamp: ev.gesture.timestamp
        )
    }

    private func makeTwoHandIntent(type: MIRHandGestureType, phase: MIRHandIntentPhase, strength: Double, confidence: Double, center: SIMD3<Double>) -> MIRHandIntent {
        let gesture = MIRHandGesture(type: type, confidence: confidence, position: center, strength: strength)
        return MIRHandIntent(
            gesture: gesture,
            phase: phase,
            position: center,
            direction: .zero,
            strength: strength,
            confidence: confidence
        )
    }
}

/// Result of one processed frame.
struct MIRHandProcessingResult: Sendable {
    let intents: [MIRHandIntent]
    let debug: MIRHandGestureDebugInfo
}

/// Snapshot of hand state for the Spatial Menu (the only thing it needs to read).
struct MIRHandSpatialContext: Sendable {
    struct HandState: Sendable {
        let handedness: Handedness
        let gesture: MIRHandGestureType
        let position: SIMD3<Double>
        let pinch: Double
        let direction: SIMD3<Double>
    }

    let hands: [HandState]
    let twoHandGesture: MIRHandGestureType

    static let empty = MIRHandSpatialContext(hands: [], twoHandGesture: .rest)
}
