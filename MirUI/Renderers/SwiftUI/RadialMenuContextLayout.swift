import Foundation
import SwiftUI

/// Runtime context of the radial menu. It follows the existing CAD event bus
/// and never owns engineering state.
@MainActor
final class RadialMenuContextStore: ObservableObject {
    static let shared = RadialMenuContextStore()

    @Published private(set) var snapshot = RadialMenuContextSnapshot(
        workbench: .model,
        hasSelection: false,
        selectionCount: 0,
        selectionKind: .none
    )

    private var subscription: UUID?

    private init() {
        subscription = MirEventBus.shared.subscribe { [weak self] event in
            guard let self else { return }
            switch event {
            case .workbenchChanged(let workbench):
                snapshot = RadialMenuContextSnapshot(
                    workbench: workbench,
                    hasSelection: snapshot.hasSelection,
                    selectionCount: snapshot.selectionCount,
                    selectionKind: snapshot.selectionKind
                )
            case .selectionChanged(let selection):
                snapshot = RadialMenuContextSnapshot(
                    workbench: snapshot.workbench,
                    hasSelection: selection.hasSelection,
                    selectionCount: selection.count,
                    selectionKind: selection.primaryKind
                )
            default:
                break
            }
        }
    }

    deinit {
        guard let subscription else {
            return
        }

        Task { @MainActor in
            MirEventBus.shared.unsubscribe(subscription)
        }
    }
}

enum RadialMenuContextLayout {
    static func panels(settings: RadialMenuSettings, context: RadialMenuContextSnapshot) -> [RadialMenuPanel] {
        let base = settings.panels.filter(\.enabled)
        let universalIDs: Set<String> = ["Файл", "Вид"]
        let universal = base.filter { universalIDs.contains($0.title) }

        let primary: RadialMenuPanel?
        switch (context.workbench, context.selectionKind) {
        case (.model, .body), (.model, .feature):
            primary = panel(
                title: "Тело",
                icon: "cube",
                tools: [
                    tool("Выдавливание", "arrow.up.to.line", "model.extrude"),
                    tool("Вращение", "arrow.triangle.2.circlepath", "model.revolve"),
                    tool("Измерение", "ruler", "measure.distance"),
                    tool("Сечение", "rectangle.split.3x3", "viewport.section")
                ]
            )
        case (.model, .face):
            primary = panel(
                title: "Поверхность",
                icon: "square.3.layers.3d",
                tools: [
                    tool("Выдавливание", "arrow.up.to.line", "model.extrude"),
                    tool("Измерение", "ruler", "measure.distance"),
                    tool("Сечение", "rectangle.split.3x3", "viewport.section"),
                    tool("Показать всё", "viewfinder", "viewport.fit")
                ]
            )
        case (.model, .edge), (.model, .vertex):
            primary = panel(
                title: "Геометрия",
                icon: "point.3.connected.trianglepath.dotted",
                tools: [
                    tool("Измерение", "ruler", "measure.distance"),
                    tool("Показать всё", "viewfinder", "viewport.fit"),
                    tool("Сечение", "rectangle.split.3x3", "viewport.section")
                ]
            )
        case (.sketch, _):
            primary = panel(
                title: "Эскиз",
                icon: "pencil.and.ruler",
                tools: [
                    tool("Линия", "line.diagonal", "sketch.line"),
                    tool("Прямоугольник", "rectangle", "sketch.rectangle"),
                    tool("Окружность", "circle", "sketch.circle"),
                    tool("Размер", "ruler", "sketch.dimension"),
                    tool("Ограничение", "link", "sketch.constraint")
                ]
            )
        case (.assembly, .component), (.assembly, .body):
            primary = panel(
                title: "Сборка",
                icon: "square.stack.3d.up",
                tools: [
                    tool("Связать", "link", "assembly.mate"),
                    tool("Пересечения", "exclamationmark.triangle", "assembly.interference"),
                    tool("Показать всё", "viewfinder", "viewport.fit")
                ]
            )
        case (.simulation, .simulationResult):
            primary = panel(
                title: "Расчёт",
                icon: "waveform.path.ecg",
                tools: [
                    tool("Запуск", "play.circle", "simulation.solve"),
                    tool("Результаты", "chart.xyaxis.line", "simulation.results")
                ]
            )
        case (.fourD, _):
            primary = panel(
                title: "4D",
                icon: "clock.arrow.circlepath",
                tools: [
                    tool("Воспроизвести", "play.fill", "fourD.play"),
                    tool("Ветка", "arrow.triangle.branch", "fourD.branch"),
                    tool("Сравнить", "square.split.2x1", "fourD.compare"),
                    tool("Что если", "questionmark.diamond", "fourD.whatIf")
                ]
            )
        case (.drawing, .drawingView):
            primary = base.first(where: { $0.title == "Чертёж" })
        case (.drawing, _):
            primary = base.first(where: { $0.title == "Чертёж" })
        default:
            primary = nil
        }

        var result: [RadialMenuPanel] = []
        if let primary { result.append(primary) }

        for panel in base where !universalIDs.contains(panel.title) && panel.title != primary?.title {
            result.append(panel)
        }
        result.append(contentsOf: universal)

        return result.isEmpty ? base : result
    }

    private static func panel(title: String, icon: String, tools: [RadialMenuTool]) -> RadialMenuPanel {
        RadialMenuPanel(title: title, icon: icon, tools: tools)
    }

    private static func tool(_ title: String, _ icon: String, _ command: String) -> RadialMenuTool {
        RadialMenuTool(title: title, icon: icon, command: command)
    }
}

/// Small preview used by settings and future contextual-menu editors.
struct RadialMenuContextPreview: View {
    let context: RadialMenuContextSnapshot
    let settings: RadialMenuSettings

    var body: some View {
        let panels = RadialMenuContextLayout.panels(settings: settings, context: context)
        VStack(alignment: .leading, spacing: 6) {
            Text(context.titleRU)
                .font(.system(size: 12, weight: .bold))
            ForEach(panels) { panel in
                HStack(spacing: 7) {
                    Image(systemName: panel.icon)
                    Text(panel.title)
                    Spacer()
                    Text("\(panel.tools.count)")
                        .foregroundStyle(.secondary)
                }
                .font(.system(size: 10))
            }
        }
    }
}
