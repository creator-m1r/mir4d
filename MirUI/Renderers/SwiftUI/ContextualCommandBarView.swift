import SwiftUI

/// Контекстная лента команд МИР 4D.
///
/// Команды не перечисляются вручную в View.
/// Панель получает ActiveContext и показывает только инженерные команды,
/// относящиеся к текущему Workbench. Глобальные команды остаются в TopBar.
struct ContextualCommandBarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry

    private let globalCommandPrefixes = [
        "document.",
        "history.",
        "viewport."
    ]

    var body: some View {
        HStack(spacing: 4) {
            ForEach(contextualCommands) { command in
                commandButton(command)
            }

            Spacer(minLength: 8)

            viewportCommand("viewport.grid")
            viewportCommand("viewport.axes")
            viewportCommand("viewport.section")
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .frame(height: 36)
        .background(MirTheme.Colors.surface.opacity(0.88))
        .overlay(alignment: .bottom) {
            Rectangle()
                .fill(MirTheme.Colors.border)
                .frame(height: 1)
        }
    }

    private var contextualCommands: [CADCommand] {
        registry.availableCommands(for: appState.activeContext)
            .filter { command in
                !globalCommandPrefixes.contains {
                    command.id.hasPrefix($0)
                }
            }
            .filter { command in
                command.id != "measure.distance"
            }
    }

    private func commandButton(_ command: CADCommand) -> some View {
        Button {
            execute(command)
        } label: {
            HStack(spacing: 5) {
                Image(systemName: command.icon)
                    .font(.system(size: 11, weight: .medium))

                Text(command.localizedTitle(appState.ui.language))
                    .font(MirTheme.Typography.caption)
                    .lineLimit(1)

                if let shortcut = command.shortcut {
                    Text(shortcut)
                        .font(MirTheme.Typography.status)
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                }
            }
            .foregroundStyle(MirTheme.Colors.textSecondary)
            .padding(.horizontal, 8)
            .frame(height: 27)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.82))
            .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain)
        .help(helpText(command))
    }

    @ViewBuilder
    private func viewportCommand(_ id: String) -> some View {
        if let command = registry.commands.first(where: { $0.id == id }) {
            let available = command.workbenches.contains(appState.workbench)
                && command.isAvailable(appState.activeContext)

            Button {
                execute(command)
            } label: {
                Image(systemName: command.icon)
                    .font(.system(size: 11, weight: .medium))
                    .foregroundStyle(
                        available
                            ? MirTheme.Colors.textSecondary
                            : MirTheme.Colors.textDisabled
                    )
                    .frame(width: 28, height: 27)
                    .background(MirTheme.Colors.surfaceRaised.opacity(0.72))
                    .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            }
            .buttonStyle(.plain)
            .disabled(!available)
            .help(helpText(command))
        }
    }

    private func execute(_ command: CADCommand) {
        guard command.workbenches.contains(appState.workbench) else { return }
        guard command.isAvailable(appState.activeContext) else { return }

        MirEventBus.shared.publish(.commandRequested(command.id))
        MirEventBus.shared.publish(.commandStarted(command.id))
        command.execute()
        MirEventBus.shared.publish(.commandFinished(command.id))
    }

    private func helpText(_ command: CADCommand) -> String {
        let title = command.localizedTitle(appState.ui.language)
        guard let shortcut = command.shortcut else { return title }
        return "\(title) · \(shortcut)"
    }
}
