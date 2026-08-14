import SwiftUI

struct ContextualToolbarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry

    var body: some View {
        let commands = Array(registry.availableCommands(for: appState.activeContext).prefix(8))

        return HStack(spacing: 4) {
            ForEach(commands) { command in
                Button {
                    execute(command)
                } label: {
                    VStack(spacing: 3) {
                        Image(systemName: command.icon)
                            .font(.system(size: 13, weight: .medium))
                        Text(command.localizedTitle(appState.ui.language))
                            .font(.system(size: 8, weight: .medium))
                            .lineLimit(1)
                    }
                    .frame(minWidth: 52, minHeight: 42)
                    .foregroundStyle(
                        isActive(command)
                            ? MirTheme.Colors.accentBright
                            : MirTheme.Colors.textSecondary
                    )
                    .background(
                        isActive(command)
                            ? MirTheme.Colors.accentSoft
                            : Color.clear
                    )
                    .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
                }
                .buttonStyle(.plain)
                .help(helpText(for: command))
            }
        }
        .padding(6)
        .mirFloating()
        .animation(MirTheme.Animation.fast, value: commands.map(\.id))
    }

    private func execute(_ command: CADCommand) {
        guard command.workbenches.contains(appState.workbench),
              command.isAvailable(appState.activeContext) else { return }

        MirEventBus.shared.publish(.commandRequested(command.id))
        MirEventBus.shared.publish(.commandStarted(command.id))
        command.execute()
        MirEventBus.shared.publish(.commandFinished(command.id))
    }

    private func isActive(_ command: CADCommand) -> Bool {
        switch command.id {
        case "viewport.select": return appState.selectedTool == "select"
        case "viewport.pan": return appState.selectedTool == "pan"
        case "viewport.zoom": return appState.selectedTool == "zoom"
        case "sketch.line": return appState.selectedTool == "line"
        case "sketch.rectangle": return appState.selectedTool == "rectangle"
        case "sketch.circle": return appState.selectedTool == "circle"
        case "sketch.constraint": return appState.selectedTool == "constraint"
        case "sketch.dimension": return appState.selectedTool == "dimension"
        case "fourD.play": return appState.workbench == .fourD && appState.isPlaying
        default: return appState.selectedTool == command.id
        }
    }

    private func helpText(for command: CADCommand) -> String {
        let title = command.localizedTitle(appState.ui.language)
        guard let shortcut = command.shortcut else { return title }
        return "\(title) · \(shortcut)"
    }
}
