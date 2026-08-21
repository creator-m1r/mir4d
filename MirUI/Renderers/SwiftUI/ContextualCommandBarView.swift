import SwiftUI

struct ContextualCommandBarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        HStack(spacing: 6) {
            Text(russian ? "Вид" : "View")
                .font(.system(size: 9, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .padding(.horizontal, 4)

            Divider().frame(height: 20)

            viewportCommand("viewport.grid")
            viewportCommand("viewport.axes")
            viewportCommand("viewport.section")

            Spacer(minLength: 8)
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
