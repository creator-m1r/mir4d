import Foundation

/// Converts the native SelectionChanged event payload into a stable Swift
/// representation for CADAppState and Inspector consumers.
struct MirSelectionResult: Equatable, Sendable {
    enum Kind: UInt64, Sendable {
        case none = 0
        case vertex = 1
        case edge = 2
        case face = 3
        case solid = 4
        case object = 5
    }

    let kind: Kind
    let id: UInt64

    static let none = MirSelectionResult(kind: .none, id: 0)
}

@MainActor
final class ViewportSelectionEventAdapter {
    static let shared = ViewportSelectionEventAdapter()

    private init() {}

    func result(kindRawValue: UInt64, id: UInt64) -> MirSelectionResult {
        MirSelectionResult(kind: MirSelectionResult.Kind(rawValue: kindRawValue) ?? .none, id: id)
    }

    func selectionKind(for filter: MirSelectionFilter) -> MirSelectionResult.Kind? {
        switch filter {
        case .auto: return nil
        case .vertex: return .vertex
        case .edge: return .edge
        case .face: return .face
        case .body: return .solid
        case .feature, .sketch: return .object
        }
    }

    /// Entry point for the native bridge. The native ABI stays untouched;
    /// consumers receive the existing CADSelectionState through MirEventBus.
    func publish(kindRawValue: UInt64, id: UInt64) {
        let result = result(kindRawValue: kindRawValue, id: id)
        MirEventBus.shared.publish(.selectionChanged(
            CADSelectionState(
                ids: result.id == 0 ? [] : [String(result.id)],
                kind: result.id == 0 ? .none : .body
            )
        ))
    }
}
