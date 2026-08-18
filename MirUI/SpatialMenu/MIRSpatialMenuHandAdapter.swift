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

    func reset() {
        active = false
        lastPosition = nil
        accumulativeDX = 0
        accumulativeDY = 0
    }

    func handle(_ intent: MIRHandIntent) {
        switch intent.gesture.type {
        case .pinch:
            let action = MIR4DInteractionContext(target: interactionTarget)
                .resolve(gesture: .pinch, phase: intent.phase)

            // Sculpting surface: emit a rich, real deformation stroke each frame
            // instead of (only) driving the radial menu. Depth is carried through.
            if interactionTarget == .sculpt,
               intent.phase == .began || intent.phase == .changed {
                MIR4DSculptIntentPublisher.shared.publish(MIR4DSculptIntent(from: intent))
                MIRIntentRouter.shared.publish(
                    MIRIntent(source: .spatial, phase: .attention, action: action.rawValue, confidence: intent.confidence)
                )
                if intent.phase == .began {
                    active = true
                    lastPosition = intent.position
                    accumulativeDX = 0
                    accumulativeDY = 0
                }
                return
            }

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
            // Sculpt surface also accepts a grab stroke (pinch released into grab).
            if interactionTarget == .sculpt, intent.phase == .began || intent.phase == .changed {
                MIR4DSculptIntentPublisher.shared.publish(MIR4DSculptIntent(from: intent))
                return
            }
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
