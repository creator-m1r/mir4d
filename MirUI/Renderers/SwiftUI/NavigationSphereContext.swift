import SwiftUI

extension Notification.Name {
    static let mir4DCameraPresetRequested = Notification.Name("MIR4D.CameraPresetRequested")
    static let mir4DCameraOrbitRequested = Notification.Name("MIR4D.CameraOrbitRequested")
}

public enum MirCameraPreset: String, CaseIterable {
    case front, back, left, right, top, bottom, isometric
    case topFrontLeft, topFrontRight, topBackLeft, topBackRight
    case bottomFrontLeft, bottomFrontRight, bottomBackLeft, bottomBackRight
    case frontLeft, frontRight, backLeft, backRight
    case topFront, topBack, topLeft, topRight
    case bottomFront, bottomBack, bottomLeft, bottomRight

    /// Stable C index used by MirEngineSetActiveCameraPreset.
    var presetIndex: Int {
        switch self {
        case .front: return 0
        case .back: return 1
        case .left: return 2
        case .right: return 3
        case .top: return 4
        case .bottom: return 5
        case .isometric: return 6
        case .topFrontLeft: return 7
        case .topFrontRight: return 8
        case .topBackLeft: return 9
        case .topBackRight: return 10
        case .bottomFrontLeft: return 11
        case .bottomFrontRight: return 12
        case .bottomBackLeft: return 13
        case .bottomBackRight: return 14
        case .frontLeft: return 15
        case .frontRight: return 16
        case .backLeft: return 17
        case .backRight: return 18
        case .topFront: return 19
        case .topBack: return 20
        case .topLeft: return 21
        case .topRight: return 22
        case .bottomFront: return 23
        case .bottomBack: return 24
        case .bottomLeft: return 25
        case .bottomRight: return 26
        }
    }

    /// Unit direction (x right, y up, z forward) used by the navigation cube
    /// to highlight the currently active view.
    var direction: SIMD3<Double> {
        switch self {
        case .front: return SIMD3(0, 0, 1)
        case .back: return SIMD3(0, 0, -1)
        case .left: return SIMD3(-1, 0, 0)
        case .right: return SIMD3(1, 0, 0)
        case .top: return SIMD3(0, 1, 0)
        case .bottom: return SIMD3(0, -1, 0)
        case .isometric: return SIMD3(1, 1, 1).normalized
        case .topFrontLeft: return SIMD3(-1, 1, 1).normalized
        case .topFrontRight: return SIMD3(1, 1, 1).normalized
        case .topBackLeft: return SIMD3(-1, 1, -1).normalized
        case .topBackRight: return SIMD3(1, 1, -1).normalized
        case .bottomFrontLeft: return SIMD3(-1, -1, 1).normalized
        case .bottomFrontRight: return SIMD3(1, -1, 1).normalized
        case .bottomBackLeft: return SIMD3(-1, -1, -1).normalized
        case .bottomBackRight: return SIMD3(1, -1, -1).normalized
        case .frontLeft: return SIMD3(-1, 0, 1).normalized
        case .frontRight: return SIMD3(1, 0, 1).normalized
        case .backLeft: return SIMD3(-1, 0, -1).normalized
        case .backRight: return SIMD3(1, 0, -1).normalized
        case .topFront: return SIMD3(0, 1, 1).normalized
        case .topBack: return SIMD3(0, 1, -1).normalized
        case .topLeft: return SIMD3(-1, 1, 0).normalized
        case .topRight: return SIMD3(1, 1, 0).normalized
        case .bottomFront: return SIMD3(0, -1, 1).normalized
        case .bottomBack: return SIMD3(0, -1, -1).normalized
        case .bottomLeft: return SIMD3(-1, -1, 0).normalized
        case .bottomRight: return SIMD3(1, -1, 0).normalized
        }
    }

    var titleRU: String {
        switch self {
        case .front: return "Спереди"
        case .back: return "Сзади"
        case .left: return "Слева"
        case .right: return "Справа"
        case .top: return "Сверху"
        case .bottom: return "Снизу"
        case .isometric: return "Изометрия"
        case .topFrontLeft: return "Верх-перед-лево"
        case .topFrontRight: return "Верх-перед-право"
        case .topBackLeft: return "Верх-зад-лево"
        case .topBackRight: return "Верх-зад-право"
        case .bottomFrontLeft: return "Низ-перед-лево"
        case .bottomFrontRight: return "Низ-перед-право"
        case .bottomBackLeft: return "Низ-зад-лево"
        case .bottomBackRight: return "Низ-зад-право"
        case .frontLeft: return "Перед-лево"
        case .frontRight: return "Перед-право"
        case .backLeft: return "Зад-лево"
        case .backRight: return "Зад-право"
        case .topFront: return "Верх-перед"
        case .topBack: return "Верх-зад"
        case .topLeft: return "Верх-лево"
        case .topRight: return "Верх-право"
        case .bottomFront: return "Низ-перед"
        case .bottomBack: return "Низ-зад"
        case .bottomLeft: return "Низ-лево"
        case .bottomRight: return "Низ-право"
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
        case .topFrontLeft: return "Top Front Left"
        case .topFrontRight: return "Top Front Right"
        case .topBackLeft: return "Top Back Left"
        case .topBackRight: return "Top Back Right"
        case .bottomFrontLeft: return "Bottom Front Left"
        case .bottomFrontRight: return "Bottom Front Right"
        case .bottomBackLeft: return "Bottom Back Left"
        case .bottomBackRight: return "Bottom Back Right"
        case .frontLeft: return "Front Left"
        case .frontRight: return "Front Right"
        case .backLeft: return "Back Left"
        case .backRight: return "Back Right"
        case .topFront: return "Top Front"
        case .topBack: return "Top Back"
        case .topLeft: return "Top Left"
        case .topRight: return "Top Right"
        case .bottomFront: return "Bottom Front"
        case .bottomBack: return "Bottom Back"
        case .bottomLeft: return "Bottom Left"
        case .bottomRight: return "Bottom Right"
        }
    }
}

extension SIMD3 where Scalar == Double {
    var normalized: SIMD3<Double> {
        let length = (x * x + y * y + z * z).squareRoot()
        guard length > 0 else { return self }
        return SIMD3(x / length, y / length, z / length)
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
