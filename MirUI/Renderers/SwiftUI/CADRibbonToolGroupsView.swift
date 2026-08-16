import SwiftUI

/// Compact industrial CAD command surface for the active workbench.
/// Commands remain owned by CADCommandRegistry; this view only groups and presents them.
struct CADRibbonToolGroupsView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(alignment: .top, spacing: 7) {
                ForEach(groups) { group in
                    ribbonGroup(group)
                }
                Spacer(minLength: 4)
            }
            .padding(.horizontal, MirTheme.Spacing.lg)
            .padding(.vertical, 5)
        }
        .frame(height: 72)
        .background(MirTheme.Colors.surface)
        .overlay(alignment: .bottom) {
            Rectangle().fill(MirTheme.Colors.border).frame(height: 1)
        }
    }

    private var groups: [RibbonGroup] {
        switch appState.workbench {
        case .model:
            return [
                RibbonGroup("Создание", "Create", ["create.body", "model.extrude", "model.revolve"]),
                RibbonGroup("Выбор", "Selection", ["viewport.select", "viewport.pan", "viewport.fit"]),
                RibbonGroup("Измерение", "Inspect", ["measure.distance"])
            ]
        case .sketch:
            return [
                RibbonGroup("Геометрия", "Geometry", ["sketch.line", "sketch.rectangle", "sketch.circle"]),
                RibbonGroup("Ограничения", "Constraints", ["sketch.constraint", "sketch.dimension"]),
                RibbonGroup("Навигация", "Navigation", ["viewport.select", "viewport.pan", "viewport.zoom", "viewport.fit"])
            ]
        case .assembly:
            return [
                RibbonGroup("Сборка", "Assembly", ["assembly.mate", "assembly.interference"]),
                RibbonGroup("Навигация", "Navigation", ["viewport.select", "viewport.pan", "viewport.fit"])
            ]
        case .simulation:
            return [
                RibbonGroup("Расчёт", "Solve", ["simulation.solve", "simulation.results"]),
                RibbonGroup("Навигация", "Navigation", ["viewport.select", "viewport.pan", "viewport.fit"])
            ]
        case .fourD:
            return [
                RibbonGroup("Время", "Time", ["fourD.play", "fourD.branch"]),
                RibbonGroup("Анализ", "Analysis", ["fourD.compare", "fourD.whatIf"]),
                RibbonGroup("Навигация", "Navigation", ["viewport.select", "viewport.pan", "viewport.fit"])
            ]
        case .drawing:
            return [RibbonGroup("Навигация", "Navigation", ["viewport.select", "viewport.pan", "viewport.fit"])]
        case .collaboration:
            return [RibbonGroup("Ревью", "Review", ["viewport.select", "viewport.pan", "viewport.fit"])]
        case .visualization:
            return [RibbonGroup("Сцена", "Scene", ["viewport.select", "viewport.pan", "viewport.fit"])]
        }
    }

    private func ribbonGroup(_ group: RibbonGroup) -> some View {
        VStack(alignment: .leading, spacing: 3) {
            HStack(spacing: 4) {
                Text(russian ? group.titleRU : group.titleEN)
                    .font(.system(size: 8, weight: .semibold))
                    .foregroundStyle(MirTheme.Colors.textTertiary)
                Spacer(minLength: 2)
            }
            HStack(spacing: 4) {
                ForEach(group.commandIDs, id: \.self) { id in
                    if let command = registry.commands.first(where: { $0.id == id }) {
                        ribbonButton(command)
                    }
                }
            }
        }
        .padding(.horizontal, 6)
        .padding(.vertical, 4)
        .background(MirTheme.Colors.surfaceRaised.opacity(0.65), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border.opacity(0.8), lineWidth: 1))
    }

    private func ribbonButton(_ command: CADCommand) -> some View {
        let available = command.workbenches.contains(appState.workbench) && command.isAvailable(appState.activeContext)
        return Button {
            guard available else { return }
            MirEventBus.shared.publish(.commandRequested(command.id))
            MirEventBus.shared.publish(.commandStarted(command.id))
            command.execute()
            MirEventBus.shared.publish(.commandFinished(command.id))
        } label: {
            VStack(spacing: 2) {
                Image(systemName: command.icon)
                    .font(.system(size: 15, weight: .medium))
                    .frame(height: 19)
                Text(command.localizedTitle(appState.ui.language))
                    .font(.system(size: 7.5, weight: .medium))
                    .lineLimit(1)
                    .frame(maxWidth: 64)
            }
            .foregroundStyle(available ? MirTheme.Colors.textSecondary : MirTheme.Colors.textDisabled)
            .frame(minWidth: 48, minHeight: 46)
            .background(available ? MirTheme.Colors.surface : MirTheme.Colors.surface.opacity(0.45), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border.opacity(0.55), lineWidth: 1))
        }
        .buttonStyle(.plain)
        .disabled(!available)
        .help(command.localizedTitle(appState.ui.language) + (command.shortcut.map { " · \($0)" } ?? ""))
    }
}

private struct RibbonGroup: Identifiable {
    let id = UUID()
    let titleRU: String
    let titleEN: String
    let commandIDs: [String]

    init(_ titleRU: String, _ titleEN: String, _ commandIDs: [String]) {
        self.titleRU = titleRU
        self.titleEN = titleEN
        self.commandIDs = commandIDs
    }
}
