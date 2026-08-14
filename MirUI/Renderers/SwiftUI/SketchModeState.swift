import SwiftUI
import Combine

/// Shared UI state for the interactive Sketch workbench.
/// The model is intentionally UI-facing; authoritative geometry remains in MirEngine.
@MainActor
final class SketchModeState: ObservableObject {
    enum Tool: String, CaseIterable, Identifiable {
        case select
        case line
        case arc
        case circle
        case rectangle
        case trim
        case offset
        case mirror
        case pattern
        case dimension

        var id: String { rawValue }
    }

    enum ConstraintKind: String, CaseIterable, Identifiable {
        case coincident
        case horizontal
        case vertical
        case parallel
        case perpendicular
        case tangent
        case equal
        case distance
        case radius
        case angle

        var id: String { rawValue }

        var title: String {
            switch self {
            case .coincident: return "Совпадение"
            case .horizontal: return "Горизонтальность"
            case .vertical: return "Вертикальность"
            case .parallel: return "Параллельность"
            case .perpendicular: return "Перпендикулярность"
            case .tangent: return "Касательность"
            case .equal: return "Равенство"
            case .distance: return "Расстояние"
            case .radius: return "Радиус"
            case .angle: return "Угол"
            }
        }
    }

    enum GeometryKind: String, Identifiable {
        case line
        case arc
        case circle
        case rectangle

        var id: String { rawValue }
    }

    struct GeometryItem: Identifiable, Equatable {
        let id: UUID
        let kind: GeometryKind
        var isConstruction = false
        var isSelected = false
    }

    struct ConstraintItem: Identifiable, Equatable {
        let id: UUID
        let kind: ConstraintKind
        var isDriving = true
    }

    @Published var isActive = false
    @Published var activeTool: Tool = .select
    @Published var activePlane = "XY"
    @Published var gridStep: Double = 10
    @Published var snapEnabled = true
    @Published var autoConstraintsEnabled = true
    @Published var geometry: [GeometryItem] = []
    @Published var constraints: [ConstraintItem] = []
    @Published var cursorWorld = CGPoint.zero
    @Published var zoom: Double = 1
    @Published var solverStatus = "Готов"
    @Published var isFullyConstrained = false
    @Published var message = "Выберите инструмент"

    var selectedGeometryCount: Int {
        geometry.filter(\.isSelected).count
    }

    var profileCandidateCount: Int {
        geometry.filter { !$0.isConstruction }.isEmpty ? 0 : 1
    }

    func enter(plane: String = "XY") {
        isActive = true
        activePlane = plane
        activeTool = .select
        message = "Эскиз активен · плоскость \(plane)"
    }

    func finish() {
        isActive = false
        activeTool = .select
        message = "Эскиз завершён"
    }

    func cancel() {
        isActive = false
        activeTool = .select
        geometry.removeAll()
        constraints.removeAll()
        message = "Построение отменено"
    }

    func choose(_ tool: Tool) {
        activeTool = tool
        message = "Инструмент: \(tool.rawValue)"
    }

    func addGeometry(_ kind: GeometryKind, construction: Bool = false) {
        geometry.append(.init(id: UUID(), kind: kind, isConstruction: construction))
        solverStatus = "Изменён"
        message = "Добавлена геометрия: \(kind.rawValue)"
    }

    func addConstraint(_ kind: ConstraintKind) {
        constraints.append(.init(id: UUID(), kind: kind))
        solverStatus = "Изменён"
        message = "Добавлено ограничение: \(kind.title)"
    }

    func clearSelection() {
        geometry = geometry.map {
            var copy = $0
            copy.isSelected = false
            return copy
        }
    }

    func toggleConstruction(for id: UUID) {
        guard let index = geometry.firstIndex(where: { $0.id == id }) else { return }
        geometry[index].isConstruction.toggle()
        solverStatus = "Изменён"
    }

    func toggleConstraint(_ id: UUID) {
        guard let index = constraints.firstIndex(where: { $0.id == id }) else { return }
        constraints[index].isDriving.toggle()
        solverStatus = "Изменён"
    }
}
