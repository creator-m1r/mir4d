import Foundation

enum MIRHandState: Sendable {
    case lost
    case detected
    case tracking
    case interacting
    case released

    var afterLoss: MIRHandState { .lost }

    func next(observed: Bool, interacting: Bool) -> MIRHandState {
        guard observed else { return .lost }
        switch self {
        case .lost:
            return .detected
        case .detected:
            return interacting ? .interacting : .tracking
        case .tracking:
            return interacting ? .interacting : .tracking
        case .interacting:
            return interacting ? .interacting : .released
        case .released:
            return interacting ? .interacting : .tracking
        }
    }
}
