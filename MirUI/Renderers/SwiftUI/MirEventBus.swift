import Foundation

@MainActor
final class MirEventBus {
    static let shared = MirEventBus()

    enum Event {
        case workbenchChanged(CADWorkbench)
        case subModeChanged(CADSubMode)
        case selectionChanged(CADSelectionState)
        case timeChanged(CADTimeState)
        case simulationChanged(CADSimulationState)
        case commandRequested(String)
        case commandStarted(String)
        case commandFinished(String)
        case commandFailed(String)
        case undoRequested
        case redoRequested
        case documentChanged
        case modelChanged
        case radialMenu(RadialMenuBusEvent)
    }

    typealias Handler = (Event) -> Void

    private var handlers: [UUID: Handler] = [:]

    private init() {}

    enum RadialMenuBusEvent: Equatable {
        case opened
        case closed
        case panelChanged(String?)
        case toolChanged(String?)
        case previewStarted(String?)
        case previewChanged(String?)
        case commandRequested(String)
        case commandConfirmed(String)
        case commandCancelled(String)
    }

    @discardableResult
    func subscribe(_ handler: @escaping Handler) -> UUID {
        let token = UUID()
        handlers[token] = handler
        return token
    }

    func unsubscribe(_ token: UUID) {
        handlers.removeValue(forKey: token)
    }

    func publish(_ event: Event) {
        for handler in handlers.values {
            handler(event)
        }
    }

    func removeAllSubscribers() {
        handlers.removeAll()
    }
}
