import Foundation
import Combine
import MirUIHandGesture

@MainActor
final class MIR4DSculptIntentPublisher {
    static let shared = MIR4DSculptIntentPublisher()

    private let subject = PassthroughSubject<MIR4DSculptIntent, Never>()

    var stream: AnyPublisher<MIR4DSculptIntent, Never> { subject.eraseToAnyPublisher() }

    func publish(_ intent: MIR4DSculptIntent) {
        subject.send(intent)
    }
}
