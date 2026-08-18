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
    private var active = false
    private var lastPosition: SIMD3<Double>?
    private var accumulativeDX: Double = 0
    private var accumulativeDY: Double = 0
    private var cancellable: AnyCancellable?

    func update(configuration: Configuration) {
        self.configuration = configuration
    }

    /// Begin consuming hand intents. Safe to call multiple times.
    func start() {
        guard cancellable == nil else { return }
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
            if intent.phase == .began || intent.phase == .changed {
                guard !active else { return }
                active = true
                lastPosition = intent.position
                accumulativeDX = 0
                accumulativeDY = 0
                MIRSpatialMenuGesture.shared.injectBegan()
                MIRIntentRouter.shared.publish(
                    MIRIntent(source: .spatial, phase: .attention, action: "spatial.menu", confidence: intent.confidence)
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
