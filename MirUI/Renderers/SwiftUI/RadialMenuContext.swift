import Foundation
import SwiftUI

/// Snapshot of the active CAD situation that decides which radial commands are meaningful.
@MainActor
struct RadialMenuContext {
    var workbench: CADWorkbench
    var subMode: CADSubMode
    var selection: CADSelectionState
    var time: CADTimeState
    var simulation: CADSimulationState
    var interaction: CADInteractionState
    var experience: CADExperience
    var language: CADLanguage

    init(appState: CADAppState) {
        self.workbench = appState.workbench
        self.subMode = appState.subMode
        self.selection = appState.selection
        self.time = appState.activeContext.time
        self.simulation = appState.simulation
        self.interaction = appState.activeContext.interaction
        self.experience = appState.activeContext.experience
        self.language = appState.activeContext.language
    }

    func allows(command: RadialMenuCommand) -> Bool {
        if command.requiresSelection, !selection.hasSelection {
            return false
        }
        if !command.workbenches.isEmpty,
           !command.workbenches.contains("all"),
           !command.workbenches.contains(String(describing: workbench)) {
            return false
        }
        if !command.selectionKinds.isEmpty,
           !command.selectionKinds.contains("any"),
           !command.selectionKinds.contains(String(describing: selection.primaryKind)) {
            return false
        }
        return true
    }
}

enum RadialMenuConfirmationKind {
    case instant
    case preview
    case destructive
}

@MainActor
struct RadialMenuCommand {
    var id: String
    var title: String
    var icon: String
    var category: String
    var requiresSelection: Bool = false
    var confirmation: RadialMenuConfirmationKind = .instant
    var workbenches: [String] = []
    var selectionKinds: [String] = []
    var description: String = ""
    var shortcut: String = ""

    var isAvailable: (RadialMenuContext) -> Bool
    var isEnabled: (RadialMenuContext) -> Bool = { _ in true }
    var isVisible: (RadialMenuContext) -> Bool = { _ in true }

    init(id: String, title: String, icon: String, category: String,
         requiresSelection: Bool = false,
         confirmation: RadialMenuConfirmationKind = .instant,
         workbenches: [String] = [], selectionKinds: [String] = [],
         description: String = "", shortcut: String = "",
         isAvailable: @escaping (RadialMenuContext) -> Bool = { _ in true }) {
        self.id = id
        self.title = title
        self.icon = icon
        self.category = category
        self.requiresSelection = requiresSelection
        self.confirmation = confirmation
        self.workbenches = workbenches
        self.selectionKinds = selectionKinds
        self.description = description
        self.shortcut = shortcut
        self.isAvailable = isAvailable
    }

    func isAvailable(_ context: RadialMenuContext) -> Bool {
        isAvailable(context) && context.allows(command: self)
    }
}

@MainActor
struct RadialCommandRegistry {
    static let shared = RadialCommandRegistry()

    let commands: [RadialMenuCommand]

    init() {
        commands = [
            RadialMenuCommand(id: "create.body", title: "Тело", icon: "cube.transparent", category: "create",
                              confirmation: .preview, workbenches: ["model", "assembly"],
                              description: "Создать твёрдое тело"),
            RadialMenuCommand(id: "create.sketch", title: "Эскиз", icon: "pencil.and.ruler", category: "create",
                              confirmation: .instant, workbenches: ["sketch"],
                              description: "Создать эскиз"),
            RadialMenuCommand(id: "create.form", title: "Форма", icon: "shape", category: "create",
                              confirmation: .preview, workbenches: ["model"]),
            RadialMenuCommand(id: "create.component", title: "Компонент", icon: "shippingbox", category: "create",
                              confirmation: .instant, workbenches: ["assembly"]),

            RadialMenuCommand(id: "modify.dimension", title: "Размер", icon: "ruler", category: "modify",
                              requiresSelection: true, confirmation: .preview,
                              description: "Изменить размер выделенного"),
            RadialMenuCommand(id: "modify.form", title: "Форма", icon: "wand.and.stars", category: "modify",
                              requiresSelection: true, confirmation: .preview),
            RadialMenuCommand(id: "transform.move", title: "Положение", icon: "arrow.up.and.down.and.arrow.left.and.arrow.right", category: "modify",
                              requiresSelection: true, confirmation: .preview),
            RadialMenuCommand(id: "modify.constraint", title: "Связь", icon: "link", category: "modify",
                              requiresSelection: true, confirmation: .instant),
            RadialMenuCommand(id: "modify.split", title: "Разделить", icon: "rectangle.split.3x1", category: "modify",
                              requiresSelection: true, confirmation: .destructive),

            RadialMenuCommand(id: "assembly.constraint", title: "Связать", icon: "link", category: "assembly",
                              requiresSelection: true, confirmation: .instant, workbenches: ["assembly"]),
            RadialMenuCommand(id: "assembly.component", title: "Собрать", icon: "square.stack.3d.up", category: "assembly",
                              confirmation: .instant, workbenches: ["assembly"]),

            RadialMenuCommand(id: "measure.distance", title: "Расстояние", icon: "arrow.left.and.right", category: "measure",
                              confirmation: .instant, description: "Измерить расстояние"),
            RadialMenuCommand(id: "measure.angle", title: "Угол", icon: "angle", category: "measure",
                              confirmation: .instant),
            RadialMenuCommand(id: "measure.dimension", title: "Размер", icon: "ruler", category: "measure",
                              confirmation: .instant),

            RadialMenuCommand(id: "view.fit", title: "Показать всё", icon: "arrow.up.left.and.arrow.down.right", category: "view",
                              confirmation: .instant),
            RadialMenuCommand(id: "view.isometric", title: "Изометрия", icon: "cube", category: "view",
                              confirmation: .instant),
            RadialMenuCommand(id: "view.orthographic", title: "Проекция", icon: "square.split.2x2", category: "view",
                              confirmation: .instant),

            RadialMenuCommand(id: "fourD.scenario", title: "Сценарий", icon: "point.3.connected.trianglepath.dotted", category: "fourD",
                              confirmation: .instant, workbenches: ["fourD"]),
            RadialMenuCommand(id: "fourD.timeline", title: "Время", icon: "clock", category: "fourD",
                              confirmation: .instant, workbenches: ["fourD"]),
            RadialMenuCommand(id: "fourD.change", title: "Изменение", icon: "waveform.path.ecg", category: "fourD",
                              confirmation: .instant, workbenches: ["fourD"]),

            RadialMenuCommand(id: "project.open", title: "Открыть", icon: "folder", category: "project",
                              confirmation: .instant),
            RadialMenuCommand(id: "file.save", title: "Сохранить", icon: "square.and.arrow.down", category: "project",
                              confirmation: .instant),
            RadialMenuCommand(id: "file.export", title: "Экспорт", icon: "square.and.arrow.up", category: "project",
                              confirmation: .instant),

            RadialMenuCommand(id: "edit.undo", title: "Отменить", icon: "arrow.uturn.backward", category: "navigation",
                              confirmation: .instant),
            RadialMenuCommand(id: "edit.redo", title: "Повторить", icon: "arrow.uturn.forward", category: "navigation",
                              confirmation: .instant),
            RadialMenuCommand(id: "navigation.close", title: "Закрыть", icon: "xmark", category: "navigation",
                              confirmation: .instant)
        ]
    }

    func command(for command: String) -> RadialMenuCommand? {
        commands.first { $0.id == command }
    }
}
