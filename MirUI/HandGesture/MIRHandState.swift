import Foundation

/// Lifecycle state of a tracked hand.
///
/// Transitions:
/// ```
/// lost → detected → tracking → interacting → released → tracking
/// ```
/// When a hand disappears the state must correctly fall back to `lost`.
enum MIRHandState: Sendable {
    case lost
    case detected
    case tracking
    case interacting
    case released

    /// Returns the next stable state after a hand is no longer observed.
    var afterLoss: MIRHandState { .lost }

    /// Advances the state machine based on whether a hand is currently observed
    /// and whether an interaction (grab / pinch / draw) is active.
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
