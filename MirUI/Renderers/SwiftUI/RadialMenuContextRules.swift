import Foundation

enum RadialMenuContextRules {
    static func selectionTitle(_ selection: CADSelectionState, language: CADLanguage = .russian) -> String {
        guard selection.hasSelection else {
            return language == .russian ? "Контекст сцены" : "Scene context"
        }

        let titleRU: String
        switch selection.primaryKind {
        case .body: titleRU = "Тело"
        case .feature: titleRU = "Элемент"
        case .sketch: titleRU = "Эскиз"
        case .face: titleRU = "Поверхность"
        case .edge: titleRU = "Ребро"
        case .vertex: titleRU = "Вершина"
        case .component: titleRU = "Компонент"
        case .simulationResult: titleRU = "Результат расчёта"
        case .drawingView: titleRU = "Вид чертежа"
        case .unknown: titleRU = "Объект"
        case .none: titleRU = "Контекст сцены"
        }

        let titleEN: String
        switch selection.primaryKind {
        case .body: titleEN = "Body"
        case .feature: titleEN = "Feature"
        case .sketch: titleEN = "Sketch"
        case .face: titleEN = "Face"
        case .edge: titleEN = "Edge"
        case .vertex: titleEN = "Vertex"
        case .component: titleEN = "Component"
        case .simulationResult: titleEN = "Simulation result"
        case .drawingView: titleEN = "Drawing view"
        case .unknown: titleEN = "Object"
        case .none: titleEN = "Scene context"
        }

        return language == .russian ? titleRU : titleEN
    }

    static func contextDescription(_ context: CADActiveContext) -> String {
        let selection = selectionTitle(context.selection, language: context.language)
        let workbench = context.language == .russian ? context.workbench.titleRU : context.workbench.titleEN
        return context.selection.hasSelection
            ? "\(workbench) · \(selection) · \(context.selection.count)"
            : "\(workbench) · \(selection)"
    }

    static func isMeaningful(command: String, context: CADActiveContext) -> Bool {
        switch command {
        case "create.body":
            return context.workbench == .model
        case "create.sketch":
            return context.workbench == .model || context.workbench == .sketch
        case "feature.extrude", "model.extrude", "feature.revolve", "model.revolve":
            return context.workbench == .model && context.selection.hasSelection
        case "transform.move":
            return context.workbench == .model && context.selection.hasSelection
        case "assembly.component", "assembly.insert":
            return context.workbench == .assembly
        case "assembly.constraint", "assembly.mate":
            return context.workbench == .assembly && context.selection.count >= 2
        case "simulation.run":
            return context.workbench == .simulation && !context.simulation.isRunning
        case "simulation.pause":
            return context.workbench == .simulation && context.simulation.isRunning
        case "simulation.results":
            return context.workbench == .simulation && context.simulation.resultSetID != nil
        case "fourD.scenario", "fourD.timeline", "fourD.branch", "fourD.compare", "fourD.whatIf":
            return context.workbench == .fourD
        case "drawing.view", "drawing.dimension", "drawing.release":
            return context.workbench == .drawing
        case "manufacturing.bom", "manufacturing.route", "manufacturing.submit":
            return context.workbench == .model || context.workbench == .assembly || context.workbench == .drawing
        case "view.fit", "view.isometric", "view.orthographic":
            return true
        case "file.import", "file.export", "file.save":
            return true
        default:
            return true
        }
    }

    static func preferredPanel(for context: CADActiveContext) -> String {
        switch context.selection.primaryKind {
        case .face, .edge, .vertex, .body, .feature, .sketch:
            return "Модель"
        case .component:
            return "Сборка"
        case .simulationResult:
            return "Симуляция"
        case .drawingView:
            return "Чертёж"
        case .none, .unknown:
            switch context.workbench {
            case .assembly: return "Сборка"
            case .simulation: return "Симуляция"
            case .fourD: return "4D"
            case .drawing: return "Чертёж"
            default: return "Модель"
            }
        }
    }
}

extension CADAppState {
    var radialMenuContextDescription: String {
        RadialMenuContextRules.contextDescription(activeContext)
    }

    var radialMenuPreferredPanel: String {
        RadialMenuContextRules.preferredPanel(for: activeContext)
    }
}
