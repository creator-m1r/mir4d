import Combine
import Foundation

/// App-layer stream of hand-drawn sketch strokes, emitted by the spatial-menu
/// hand adapter while the Sketch workbench is active.
@MainActor
final class MIR4DSketchIntentPublisher {
    static let shared = MIR4DSketchIntentPublisher()

    private let subject = PassthroughSubject<MIR4DSketchIntent, Never>()

    var stream: AnyPublisher<MIR4DSketchIntent, Never> { subject.eraseToAnyPublisher() }

    func publish(_ intent: MIR4DSketchIntent) {
        subject.send(intent)
    }
}
