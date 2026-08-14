import SwiftUI

extension Notification.Name {
    static let mir4DCameraPresetRequested = Notification.Name("MIR4D.CameraPresetRequested")
}

public enum MirCameraPreset: String, CaseIterable {
    case front, back, left, right, top, bottom, isometric

    var titleRU: String {
        switch self {
        case .front: return "Спереди"
        case .back: return "Сзади"
        case .left: return "Слева"
        case .right: return "Справа"
        case .top: return "Сверху"
        case .bottom: return "Снизу"
        case .isometric: return "Изометрия"
        }
    }

    var titleEN: String {
        switch self {
        case .front: return "Front"
        case .back: return "Back"
        case .left: return "Left"
        case .right: return "Right"
        case .top: return "Top"
        case .bottom: return "Bottom"
        case .isometric: return "Isometric"
        }
    }
}

struct RadialMenuContextSnapshot: Equatable {
    var workbench: CADWorkbench
    var hasSelection: Bool
    var selectionCount: Int
    var selectionKind: CADSelectionKind = .none

    var titleRU: String {
        if hasSelection {
            switch selectionKind {
            case .vertex: return "Вершина"
            case .edge: return "Ребро"
            case .face: return "Поверхность"
            case .body: return "Тело"
            case .feature: return "Элемент"
            case .sketch: return "Эскиз"
            case .component: return "Компонент"
            case .simulationResult: return "Результат"
            case .drawingView: return "Вид чертежа"
            default: break
            }
        }

        switch workbench {
        case .model: return "Сцена"
        case .sketch: return "Эскиз"
        case .assembly: return "Сборка"
        case .simulation: return "Симуляция"
        case .fourD: return "4D"
        case .drawing: return "Чертёж"
        case .collaboration: return "Сотрудничество"
        case .visualization: return "Визуализация"
        }
    }
}

private struct RadialMenuContextKey: EnvironmentKey {
    static let defaultValue = RadialMenuContextSnapshot(
        workbench: .model,
        hasSelection: false,
        selectionCount: 0,
        selectionKind: .none
    )
}

extension EnvironmentValues {
    var radialMenuContext: RadialMenuContextSnapshot {
        get { self[RadialMenuContextKey.self] }
        set { self[RadialMenuContextKey.self] = newValue }
    }
}
