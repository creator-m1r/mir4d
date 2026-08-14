import SwiftUI

struct CommandPaletteView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry
    @Environment(\.dismiss) private var dismiss
    @State private var query = ""
    @FocusState private var searchFocused: Bool

    private var filteredCommands: [CADCommand] {
        let available = registry.availableCommands(for: appState.activeContext)
        let normalized = query.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()

        guard !normalized.isEmpty else { return available }

        return available.filter { command in
            command.id.lowercased().contains(normalized)
                || command.titleRU.lowercased().contains(normalized)
                || command.titleEN.lowercased().contains(normalized)
        }
    }

    var body: some View {
        VStack(spacing: 0) {
            searchField
            contextHeader
            Divider().opacity(0.35)

            if filteredCommands.isEmpty {
                VStack(spacing: 8) {
                    Image(systemName: "magnifyingglass")
                        .font(.system(size: 24))
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                    Text(appState.ui.language == .russian ? "Команды не найдены" : "No commands found")
                        .font(MirTheme.Typography.body)
                        .foregroundStyle(MirTheme.Colors.textSecondary)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
            } else {
                ScrollView {
                    LazyVStack(spacing: 2) {
                        ForEach(filteredCommands) { command in
                            commandRow(command)
                        }
                    }
                    .padding(8)
                }
            }
        }
        .frame(width: 580, height: 500)
        .background(MirTheme.Colors.panel)
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.large))
        .overlay {
            RoundedRectangle(cornerRadius: MirTheme.Radius.large)
                .stroke(MirTheme.Colors.borderStrong, lineWidth: 1)
        }
        .shadow(radius: 30, y: 12)
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
            searchFocused = true
        }
        .onSubmit {
            executeFirstCommand()
        }
    }

    private var searchField: some View {
        HStack(spacing: 10) {
            Image(systemName: "command")
                .foregroundStyle(MirTheme.Colors.accentBright)

            TextField(
                appState.ui.language == .russian ? "Поиск команд, объектов и действий…" : "Search commands, objects and actions…",
                text: $query
            )
            .textFieldStyle(.plain)
            .font(MirTheme.Typography.body)
            .foregroundStyle(MirTheme.Colors.textPrimary)
            .focused($searchFocused)

            Text("ESC")
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
        }
        .padding(14)
    }

    private var contextHeader: some View {
        HStack(spacing: 8) {
            Text(appState.ui.language == .russian ? appState.workbench.titleRU : appState.workbench.titleEN)
            Text("·")
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text(appState.ui.language == .russian ? appState.subMode.titleRU : appState.subMode.titleEN)

            Spacer()

            Text(appState.ui.experience == .expert ? "EXP" : "BAS")
                .font(.system(size: 9, weight: .bold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.accentBright)
        }
        .font(MirTheme.Typography.caption)
        .foregroundStyle(MirTheme.Colors.textSecondary)
        .padding(.horizontal, 14)
        .padding(.bottom, 10)
    }

    private func commandRow(_ command: CADCommand) -> some View {
        Button {
            execute(command)
        } label: {
            HStack(spacing: 12) {
                Image(systemName: command.icon)
                    .frame(width: 24)
                    .foregroundStyle(MirTheme.Colors.accentBright)

                VStack(alignment: .leading, spacing: 2) {
                    Text(command.localizedTitle(appState.ui.language))
                        .font(MirTheme.Typography.bodyMedium)
                        .foregroundStyle(MirTheme.Colors.textPrimary)

                    Text(command.id)
                        .font(.system(size: 9, design: .monospaced))
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                }

                Spacer()

                if let shortcut = command.shortcut {
                    Text(shortcut)
                        .font(.system(size: 10, weight: .medium, design: .monospaced))
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                        .padding(.horizontal, 7)
                        .padding(.vertical, 4)
                        .background(MirTheme.Colors.border.opacity(0.45))
                        .clipShape(RoundedRectangle(cornerRadius: 4))
                }
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 8)
            .background(Color.clear)
            .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain)
    }

    private func execute(_ command: CADCommand) {
        guard command.workbenches.contains(appState.workbench),
              command.isAvailable(appState.activeContext) else { return }

        MirEventBus.shared.publish(.commandRequested(command.id))
        MirEventBus.shared.publish(.commandStarted(command.id))
        command.execute()
        MirEventBus.shared.publish(.commandFinished(command.id))
        dismiss()
    }

    private func executeFirstCommand() {
        guard let command = filteredCommands.first else { return }
        execute(command)
    }
}
