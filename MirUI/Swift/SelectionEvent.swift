import Foundation

public enum SelectionEventKind: UInt64, Equatable, Sendable {
    case none = 0
    case vertex = 1
    case edge = 2
    case face = 3
    case solid = 4
    case object = 5
}

public struct SelectionEvent: Equatable, Sendable {
    public let kind: SelectionEventKind
    public let id: UInt64

    public init(kind: SelectionEventKind, id: UInt64) {
        self.kind = kind
        self.id = id
    }
}

public extension SelectionInspector {

    func receiveSelectionEvent(_ event: SelectionEvent) {
        if event.kind == .none || event.id == 0 {
            clear()
        }
    }
}
