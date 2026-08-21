import Foundation

/// UI-side description of a geometry parameter edit.
/// Execution must be delegated to MirEngine's command/solver layer.
struct SketchGeometryEditCommand: Identifiable, Equatable {
    let id = UUID()
    let geometryID: UUID
    let parameter: Parameter
    let value: Double

    enum Parameter: String, Equatable {
        case x1
        case y1
        case x2
        case y2
        case length
        case radius
        case angle
    }
}

@MainActor
final class SketchGeometryEditQueue: ObservableObject {
    @Published private(set) var pending: [SketchGeometryEditCommand] = []

    func enqueue(_ command: SketchGeometryEditCommand) {
        pending.append(command)
    }

    func remove(_ command: SketchGeometryEditCommand) {
        pending.removeAll { $0.id == command.id }
    }

    func clear() {
        pending.removeAll()
    }
}
