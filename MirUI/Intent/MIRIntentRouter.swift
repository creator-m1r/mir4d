import Foundation
import Combine

/// Central transport for device-independent engineering intent.
/// It deliberately does not execute CAD commands; consumers resolve intent against context.
@MainActor
final class MIRIntentRouter: ObservableObject {
    static let shared = MIRIntentRouter()

    @Published private(set) var latestIntent: MIRIntent?

    private let subject = PassthroughSubject<MIRIntent, Never>()

    var publisher: AnyPublisher<MIRIntent, Never> {
        subject.eraseToAnyPublisher()
    }

    func publish(_ intent: MIRIntent) {
        latestIntent = intent
        subject.send(intent)
    }

    func clear() {
        latestIntent = nil
    }
}
