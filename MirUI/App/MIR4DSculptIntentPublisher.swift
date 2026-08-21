import Foundation
import Combine
import MirUIHandGesture

/// App-layer stream of rich sculpt strokes, emitted by the hand module while the
/// interaction target is `.sculpt`, and consumed by `MIR4DSculptCommandBridge`
/// (→ MirEngine mesh deformation).
///
/// Kept separate from the canonical `MIRIntentRouter` so the bus stays
/// primitive-typed (§33 / §11); only the compact `MIRIntent(action: "sculpt")`
/// travels on the bus.
@MainActor
final class MIR4DSculptIntentPublisher {
    static let shared = MIR4DSculptIntentPublisher()

    private let subject = PassthroughSubject<MIR4DSculptIntent, Never>()

    var stream: AnyPublisher<MIR4DSculptIntent, Never> { subject.eraseToAnyPublisher() }

    func publish(_ intent: MIR4DSculptIntent) {
        subject.send(intent)
    }
}
