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
                applyVisibleContext()
            case .selectionChanged(let selection):
                snapshot = RadialMenuContextSnapshot(
                    workbench: snapshot.workbench,
                    hasSelection: selection.hasSelection,
                    selectionCount: selection.count,
                    selectionKind: selection.primaryKind
                )
                applyVisibleContext()
            default:
                break
            }
        }
        applyVisibleContext()
    }

    /// The visual radial menu is intentionally driven by the same context
    /// policy as the availability layer. This keeps the scene, gesture and
    /// command vocabulary synchronized without duplicating context logic in
    /// RadialMenuView.
    private func applyVisibleContext() {
        let store = RadialMenuSettingsStore.shared
        store.settings.panels = RadialMenuContextLayout.panels(
            settings: store.settings,
            context: snapshot
        )
    }

    deinit {
        guard let subscription else { return }
        Task { @MainActor in MirEventBus.shared.unsubscribe(subscription) }
    }
}

enum RadialMenuContextLayout {
    /// Builds the visible hierarchy without changing the user's saved menu.
    /// The first position is always the most relevant action family for the
    /// current engineering context; navigation and scene-wide actions remain
    /// available at the outer edge.
    static func panels(settings: RadialMenuSettings, context: RadialMenuContextSnapshot) -> [RadialMenuPanel] {
        let base = settings.panels.filter(\.enabled)
        guard !base.isEmpty else { return [] }

        let universalTitles: Set<String> = ["Проект", "Посмотреть", "Назад", "Файл", "Вид"]
        let universal = base.filter { universalTitles.contains($0.title) }
        let universalIDs = Set(universal.map(\.id))

        let primary: RadialMenuPanel?
        switch (context.workbench, context.selectionKind) {
        case (.model, .body), (.model, .feature):
            primary = panel("Тело", "cube", [
                tool("Изменить", "slider.horizontal.3", "modify.form"),
                tool("Переместить", "arrow.up.and.down.and.arrow.left.and.arrow.right", "transform.move"),
                tool("Измерить", "ruler", "measure.distance"),
                tool("Сечение", "rectangle.split.3x3", "viewport.section")
            ])
        case (.model, .face):
            primary = panel("Поверхность", "square.3.layers.3d", [
                tool("Изменить", "wand.and.stars", "modify.form"),
                tool("Размер", "ruler", "measure.distance"),
                tool("Сечение", "rectangle.split.3x3", "viewport.section"),
                tool("Показать всё", "viewfinder", "view.fit")
            ])
        case (.model, .edge), (.model, .vertex):
            primary = panel("Геометрия", "point.3.connected.trianglepath.dotted", [
                tool("Измерить", "ruler", "measure.distance"),
                tool("Продолжить", "arrow.right", "geometry.continue"),
                tool("Показать всё", "viewfinder", "view.fit")
            ])
        case (.sketch, _):
            primary = panel("Эскиз", "pencil.and.ruler", [
                tool("Нарисовать", "pencil", "sketch.line"),
                tool("Размер", "ruler", "sketch.dimension"),
                tool("Привязать", "scope", "sketch.constraint"),
                tool("Изменить", "slider.horizontal.3", "sketch.edit"),
                tool("Завершить", "checkmark", "mode.finish")
            ])
        case (.assembly, .component), (.assembly, .body):
            primary = panel("Сборка", "square.stack.3d.up", [
                tool("Соединить", "link", "assembly.mate"),
                tool("Переместить", "arrow.up.and.down.and.arrow.left.and.arrow.right", "transform.move"),
                tool("Проверить", "exclamationmark.triangle", "assembly.interference"),
                tool("Показать всё", "viewfinder", "view.fit")
            ])
        case (.simulation, .simulationResult):
            primary = panel("Расчёт", "waveform.path.ecg", [
                tool("Запустить", "play.circle", "simulation.solve"),
                tool("Результаты", "chart.xyaxis.line", "simulation.results")
            ])
        case (.fourD, _):
            primary = panel("Время", "clock.arrow.circlepath", [
                tool("Воспроизвести", "play.fill", "fourD.play"),
                tool("Ветка", "arrow.triangle.branch", "fourD.branch"),
                tool("Сравнить", "square.split.2x1", "fourD.compare"),
                tool("Что если", "questionmark.diamond", "fourD.whatIf")
            ])
        case (.drawing, _):
            primary = base.first(where: { $0.title == "Чертёж" })
        default:
            primary = nil
        }

        var result: [RadialMenuPanel] = []
        if let primary { result.append(primary) }

        for panel in base where panel.id != primary?.id && !universalIDs.contains(panel.id) {
            result.append(panel)
        }
        result.append(contentsOf: universal)

        return result
    }

    private static func panel(_ title: String, _ icon: String, _ tools: [RadialMenuTool]) -> RadialMenuPanel {
        RadialMenuPanel(title: title, icon: icon, tools: tools)
    }

    private static func tool(_ title: String, _ icon: String, _ command: String) -> RadialMenuTool {
        RadialMenuTool(title: title, icon: icon, command: command)
    }
}

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
