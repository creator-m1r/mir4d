import Foundation
import SwiftUI

@MainActor
struct CADCommand: Identifiable {
    let id: String
    let titleRU: String
    let titleEN: String
    let icon: String
    let shortcut: String?
    let workbenches: Set<CADWorkbench>
    let isAvailable: (CADActiveContext) -> Bool
    let execute: () -> Void

    var title: String {
        titleRU
    }

    func localizedTitle(_ language: CADLanguage) -> String {
        language == .russian ? titleRU : titleEN
    }
}

@MainActor
final class CADCommandRegistry: ObservableObject {
    @Published private(set) var commands: [CADCommand] = []

    private var registered = false

    func register(_ command: CADCommand) {
        guard !commands.contains(where: { $0.id == command.id }) else { return }
        commands.append(command)
    }

    func execute(id: String, context: CADActiveContext) -> Bool {
        guard let command = commands.first(where: { $0.id == id }),
              command.workbenches.contains(context.workbench),
              command.isAvailable(context),
              RadialMenuContextPolicyStore.shared.isAllowed(id, context: context) else {
            return false
        }
        command.execute()
        return true
    }

    func localizedTitle(for id: String, language: CADLanguage) -> String {
        guard let command = commands.first(where: { $0.id == id }) else { return id }
        return command.localizedTitle(language)
    }

    func registerDefaults(appState: CADAppState) {
        guard !registered else { return }
        registered = true

        register(CADCommand(
            id: "document.new",
            titleRU: "Новый проект",
            titleEN: "New Project",
            icon: "doc.badge.plus",
            shortcut: "⌘N",
            workbenches: Set(CADWorkbench.allCases),
            isAvailable: { _ in true },
            execute: { appState.newDocument() }
        ))
        register(CADCommand(
            id: "create.body",
            titleRU: "Создать тело",
            titleEN: "Create Body",
            icon: "cube.transparent",
            shortcut: nil,
            workbenches: Set(CADWorkbench.allCases),
            isAvailable: { _ in MIR4DProjectSession.shared.projectURL != nil },
            execute: {
                _ = MIR4DModelCommands.shared.createBox(
                    appState: appState,
                    width: 100,
                    depth: 60,
                    height: 40
                )
            }
        ))
        register(CADCommand(
            id: "history.undo",
            titleRU: "Отменить",
            titleEN: "Undo",
            icon: "arrow.uturn.backward",
            shortcut: "⌘Z",
            workbenches: Set(CADWorkbench.allCases),
            isAvailable: { _ in true },
            execute: { MirEventBus.shared.publish(.undoRequested) }
        ))
        register(CADCommand(
            id: "history.redo",
            titleRU: "Повторить",
            titleEN: "Redo",
            icon: "arrow.uturn.forward",
            shortcut: "⇧⌘Z",
            workbenches: Set(CADWorkbench.allCases),
            isAvailable: { _ in true },
            execute: { MirEventBus.shared.publish(.redoRequested) }
        ))
        register(CADCommand(
            id: "viewport.select",
            titleRU: "Выбор",
            titleEN: "Select",
            icon: "cursorarrow",
            shortcut: "V",
            workbenches: Set(CADWorkbench.allCases),
            isAvailable: { _ in true },
            execute: { appState.selectedTool = "select" }
        ))
        register(CADCommand(
            id: "viewport.pan",
            titleRU: "Панорама",
            titleEN: "Pan",
            icon: "hand.draw",
            shortcut: "P",
            workbenches: Set(CADWorkbench.allCases),
            isAvailable: { _ in true },
            execute: { appState.selectedTool = "pan" }
        ))
        register(CADCommand(
            id: "viewport.zoom",
            titleRU: "Масштаб",
            titleEN: "Zoom",
            icon: "magnifyingglass",
            shortcut: "Z",
            workbenches: Set(CADWorkbench.allCases),
            isAvailable: { _ in true },
            execute: { appState.selectedTool = "zoom" }
        ))
        register(CADCommand(
            id: "viewport.fit",
            titleRU: "Показать всё",
            titleEN: "Fit All",
            icon: "viewfinder",
            shortcut: "F",
            workbenches: Set(CADWorkbench.allCases),
            isAvailable: { _ in true },
            execute: { MirEventBus.shared.publish(.commandRequested("viewport.fit")) }
        ))
        register(CADCommand(
            id: "viewport.grid",
            titleRU: "Сетка",
            titleEN: "Grid",
            icon: "grid",
            shortcut: nil,
            workbenches: [.model, .sketch, .drawing],
            isAvailable: { _ in true },
            execute: { appState.toggleGrid() }
        ))
        register(CADCommand(
            id: "viewport.axes",
            titleRU: "Оси",
            titleEN: "Axes",
            icon: "cube",
            shortcut: nil,
            workbenches: [.model, .sketch, .assembly, .fourD],
            isAvailable: { _ in true },
            execute: { appState.toggleAxes() }
        ))
        register(CADCommand(
            id: "viewport.section",
            titleRU: "Сечение",
            titleEN: "Section",
            icon: "rectangle.split.3x3",
            shortcut: "X",
            workbenches: [.model, .assembly, .fourD, .simulation],
            isAvailable: { context in context.selection.hasSelection },
            execute: { appState.toggleSection() }
        ))
        register(CADCommand(
            id: "measure.distance",
            titleRU: "Измерение",
            titleEN: "Measure",
            icon: "ruler",
            shortcut: "D",
            workbenches: [.model, .sketch, .assembly, .drawing],
            isAvailable: { context in context.selection.hasSelection },
            execute: { appState.selectedTool = "measure" }
        ))
        register(CADCommand(
            id: "sketch.line",
            titleRU: "Линия",
            titleEN: "Line",
            icon: "line.diagonal",
            shortcut: "L",
            workbenches: [.sketch],
            isAvailable: { context in context.workbench == .sketch },
            execute: { appState.selectWorkbench(.sketch); appState.selectedTool = "line" }
        ))
        register(CADCommand(
            id: "sketch.rectangle",
            titleRU: "Прямоугольник",
            titleEN: "Rectangle",
            icon: "rectangle",
            shortcut: "R",
            workbenches: [.sketch],
            isAvailable: { context in context.workbench == .sketch },
            execute: { appState.selectWorkbench(.sketch); appState.selectedTool = "rectangle" }
        ))
        register(CADCommand(
            id: "sketch.circle",
            titleRU: "Окружность",
            titleEN: "Circle",
            icon: "circle",
            shortcut: "C",
            workbenches: [.sketch],
            isAvailable: { context in context.workbench == .sketch },
            execute: { appState.selectWorkbench(.sketch); appState.selectedTool = "circle" }
        ))
        register(CADCommand(
            id: "sketch.constraint",
            titleRU: "Добавить ограничение",
            titleEN: "Add Constraint",
            icon: "link",
            shortcut: nil,
            workbenches: [.sketch],
            isAvailable: { context in context.workbench == .sketch && context.selection.count >= 1 },
            execute: { appState.selectedTool = "constraint"; MirEventBus.shared.publish(.commandRequested("sketch.constraint")) }
        ))
        register(CADCommand(
            id: "sketch.dimension",
            titleRU: "Задать размер",
            titleEN: "Add Dimension",
            icon: "ruler",
            shortcut: "D",
            workbenches: [.sketch],
            isAvailable: { context in context.workbench == .sketch && context.selection.count >= 1 },
            execute: { appState.selectedTool = "dimension"; MirEventBus.shared.publish(.commandRequested("sketch.dimension")) }
        ))
        register(CADCommand(
            id: "model.extrude",
            titleRU: "Выдавливание",
            titleEN: "Extrude",
            icon: "arrow.up.to.line",
            shortcut: nil,
            workbenches: [.model],
            isAvailable: { context in context.workbench == .model && context.selection.hasSelection },
            execute: { MirEventBus.shared.publish(.commandRequested("model.extrude")) }
        ))
        register(CADCommand(
            id: "model.revolve",
            titleRU: "Вращение",
            titleEN: "Revolve",
            icon: "arrow.triangle.2.circlepath",
            shortcut: nil,
            workbenches: [.model],
            isAvailable: { context in context.workbench == .model && context.selection.hasSelection },
            execute: { MirEventBus.shared.publish(.commandRequested("model.revolve")) }
        ))
        register(CADCommand(
            id: "model.sculpt",
            titleRU: "Воздушный скульпт",
            titleEN: "Air Sculpt",
            icon: "wand.and.rays",
            shortcut: nil,
            workbenches: [.model],
            isAvailable: { context in context.workbench == .model && context.selection.hasSelection },
            execute: {
                appState.selectedTool = "sculpt"
                MirEventBus.shared.publish(.commandRequested("model.sculpt"))
            }
        ))
        register(CADCommand(
            id: "assembly.mate",
            titleRU: "Связать детали",
            titleEN: "Mate Components",
            icon: "link",
            shortcut: nil,
            workbenches: [.assembly],
            isAvailable: { context in context.workbench == .assembly && context.selection.count >= 2 },
            execute: { MirEventBus.shared.publish(.commandRequested("assembly.mate")) }
        ))
        register(CADCommand(
            id: "assembly.interference",
            titleRU: "Проверить пересечения",
            titleEN: "Check Interference",
            icon: "exclamationmark.triangle",
            shortcut: nil,
            workbenches: [.assembly],
            isAvailable: { context in context.workbench == .assembly },
            execute: { MirEventBus.shared.publish(.commandRequested("assembly.interference")) }
        ))
        register(CADCommand(
            id: "simulation.solve",
            titleRU: "Запустить расчёт",
            titleEN: "Run Simulation",
            icon: "play.circle",
            shortcut: "⌘R",
            workbenches: [.simulation],
            isAvailable: { context in context.workbench == .simulation && !context.simulation.isRunning },
            execute: { MirEventBus.shared.publish(.commandRequested("simulation.solve")); appState.simulation.phase = .solve; appState.simulation.isRunning = true; appState.simulation.progress = 0 }
        ))
        register(CADCommand(
            id: "simulation.results",
            titleRU: "Показать результаты",
            titleEN: "Show Results",
            icon: "chart.xyaxis.line",
            shortcut: nil,
            workbenches: [.simulation],
            isAvailable: { context in context.workbench == .simulation && context.simulation.resultSetID != nil },
            execute: { appState.simulation.phase = .results; MirEventBus.shared.publish(.commandRequested("simulation.results")) }
        ))
        register(CADCommand(
            id: "fourD.play",
            titleRU: "Воспроизвести время",
            titleEN: "Play Time",
            icon: "play.fill",
            shortcut: "Space",
            workbenches: [.fourD],
            isAvailable: { _ in true },
            execute: { appState.togglePlayback() }
        ))
        register(CADCommand(
            id: "fourD.branch",
            titleRU: "Создать ветку сценария",
            titleEN: "Create Scenario Branch",
            icon: "arrow.triangle.branch",
            shortcut: nil,
            workbenches: [.fourD],
            isAvailable: { context in context.workbench == .fourD },
            execute: { appState.createTimeBranch(); MirEventBus.shared.publish(.commandRequested("fourD.branch")) }
        ))
        register(CADCommand(
            id: "fourD.compare",
            titleRU: "Сравнить состояния",
            titleEN: "Compare States",
            icon: "square.split.2x1",
            shortcut: nil,
            workbenches: [.fourD],
            isAvailable: { context in context.workbench == .fourD },
            execute: { appState.subMode = .fourDCompare; MirEventBus.shared.publish(.commandRequested("fourD.compare")) }
        ))
        register(CADCommand(
            id: "fourD.whatIf",
            titleRU: "Что если",
            titleEN: "What-if",
            icon: "questionmark.diamond",
            shortcut: nil,
            workbenches: [.fourD],
            isAvailable: { context in context.workbench == .fourD },
            execute: { appState.subMode = .fourDWhatIf; MirEventBus.shared.publish(.commandRequested("fourD.whatIf")) }
        ))
    }

    func availableCommands(for context: CADActiveContext) -> [CADCommand] {
        commands.filter {
            $0.workbenches.contains(context.workbench) &&
            $0.isAvailable(context) &&
            RadialMenuContextPolicyStore.shared.isAllowed($0.id, context: context)
        }
    }
}
