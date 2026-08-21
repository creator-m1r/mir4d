import MirUIHandGesture
import Foundation
import CoreGraphics
import Combine

/// Bridges hand intents into the Spatial Menu fan.
///
/// Per the agent contract, the hand module and the Spatial Menu communicate
/// **only** through `MIRHandIntent` (carried on `MIRIntentRouter`). This adapter
/// subscribes to the hand module's intent stream and drives the same single
/// event path the trackpad uses:
/// ```text
/// ✋ → menu centre → move right → EDIT → move up → MOVE → move right → AXIS
/// ```
@MainActor
final class MIRSpatialMenuHandAdapter: ObservableObject {
    static let shared = MIRSpatialMenuHandAdapter()

    struct Configuration: Equatable, Sendable {
        var scale: CGFloat = 420
        var minimumMotion: CGFloat = 6
    }

    private(set) var configuration = Configuration()
    /// Current scene context the hand interaction is performed over. Supplied by
    /// the Spatial Menu controller from the live selection; defaults to `.empty`.
    private(set) var interactionTarget: MIR4DInteractionTarget = .empty
    private var active = false
    private var lastPosition: SIMD3<Double>?
    private var accumulativeDX: Double = 0
    private var accumulativeDY: Double = 0
    private var cancellable: AnyCancellable?

    /// In-progress hand-drawn sketch stroke (normalized -1…1 vertices).
    private var sketchTrail: [CGPoint] = []

    func update(configuration: Configuration) {
        self.configuration = configuration
    }

    /// Feed the live scene context so gestures resolve to semantic actions.
    func setInteractionTarget(_ target: MIR4DInteractionTarget) {
        interactionTarget = target
    }

    /// Begin consuming hand intents. Safe to call multiple times.
    func start() {
        guard cancellable == nil else { return }
        // Ensure the sculpt bridge is subscribed for the lifetime of the session.
        _ = MIR4DSculptCommandBridge.shared
        // Ensure the sketch bridge is subscribed for the lifetime of the session.
        _ = MIR4DSketchCommandBridge.shared
        cancellable = MIRHandGestureModule.shared.session.intentPublisher
            .receive(on: DispatchQueue.main)
            .sink { [weak self] intent in
                self?.handle(intent)
            }
    }

    func stop() {
        cancellable?.cancel()
        cancellable = nil
        reset()
    }

    /// Draws a freeform stroke while the Sketch workbench is active. A pointing
    /// hand accumulates vertices; releasing commits the stroke. Each frame is
    /// published (live preview) so the bridge can show the stroke in real time.
    private func handleSketch(_ intent: MIRHandIntent) {
        guard intent.gesture.type == .point else { return }
        let vertex = CGPoint(x: intent.position.x, y: intent.position.y)

        switch intent.phase {
        case .began:
            sketchTrail = [vertex]
            MIR4DSketchIntentPublisher.shared.publish(MIR4DSketchIntent(points: sketchTrail, live: true))
        case .changed:
            sketchTrail.append(vertex)
            MIR4DSketchIntentPublisher.shared.publish(MIR4DSketchIntent(points: sketchTrail, live: true))
        case .ended, .cancelled:
            if sketchTrail.count >= 2 {
                MIR4DSketchIntentPublisher.shared.publish(MIR4DSketchIntent(points: sketchTrail, live: false))
            }
            sketchTrail.removeAll()
        }
    }

    func reset() {
        active = false
        lastPosition = nil
        accumulativeDX = 0
        accumulativeDY = 0
    }

    func handle(_ intent: MIRHandIntent) {
        let action = MIR4DInteractionContext(target: interactionTarget)
            .resolve(gesture: .pinch, phase: intent.phase)

        // Sculpt surface: every supported hand pose deforms the selected body in
        // a distinct mode and publishes on the App stream; the radial menu is not
        // navigated while sculpting. `active` is intentionally left untouched so
        // menu navigation still works after leaving sculpt mode.
        if interactionTarget == .sculpt {
            switch intent.phase {
            case .began, .changed:
                if let mode = MIR4DSculptIntent.Mode(handPose: intent.gesture.type) {
                    var sculpt = MIR4DSculptIntent(from: intent)
                    sculpt.mode = mode
                    MIR4DSculptIntentPublisher.shared.publish(sculpt)
                }
                MIRIntentRouter.shared.publish(
                    MIRIntent(source: .spatial, phase: .attention, action: action.rawValue, confidence: intent.confidence)
                )
            case .ended, .cancelled:
                break
            }
            return
        }

        // Sketch workbench: a pointing hand draws a freeform stroke on the active
        // plane. The radial menu is still opened with a pinch.
        if interactionTarget == .sketch {
            handleSketch(intent)
            return
        }

        switch intent.gesture.type {
        case .pinch:
            if intent.phase == .began || intent.phase == .changed {
                guard !active else { return }
                active = true
                lastPosition = intent.position
                accumulativeDX = 0
                accumulativeDY = 0
                MIRSpatialMenuGesture.shared.injectBegan()
                MIRIntentRouter.shared.publish(
                    MIRIntent(source: .spatial, phase: .attention, action: action.rawValue, confidence: intent.confidence)
                )
            } else if intent.phase == .ended || intent.phase == .cancelled {
                guard active else { return }
                MIRSpatialMenuGesture.shared.injectEnded(commit: true)
                reset()
            }
        case .point, .grab:
            guard active, let last = lastPosition else { return }
            let dx = (intent.position.x - last.x) * Double(configuration.scale)
            let dy = (intent.position.y - last.y) * Double(configuration.scale)
            lastPosition = intent.position
            guard hypot(dx, dy) >= Double(configuration.minimumMotion) else { return }
            accumulativeDX += dx
            accumulativeDY += dy
            MIRSpatialMenuGesture.shared.injectMoved(dx: accumulativeDX, dy: accumulativeDY)
        default:
            break
        }
    }
}
