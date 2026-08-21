import SwiftUI

/// Transient action bar for the current CAD selection/context.
///
/// The viewport itself stays visually quiet. Commands are resolved through the
/// existing CADCommandRegistry/EventBus instead of creating a second command system.
struct CADViewportContextActionBar: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry
    var onCommandPalette: (() -> Void)? = nil

    private var ru: Bool { appState.ui.language == .russian }
    private var hasSelection: Bool { appState.selection.hasSelection }
    private var count: Int { appState.selectionCount }

    private var commands: [CADCommand] {
        guard hasSelection else { return [] }
        return Array(registry.availableCommands(for: appState.activeContext)
            .filter { $0.workbenches.contains(appState.workbench) && $0.isAvailable(appState.activeContext) }
            .prefix(4))
    }

    var body: some View {
        if hasSelection {
            HStack(spacing: 6) {
                selectionBadge
                Divider().frame(height: 22)

                ForEach(commands) { command in
                    commandButton(command)
                }

                if commands.isEmpty == false {
                    Divider().frame(height: 22)
                }

                Menu {
                    Button { onCommandPalette?() } label: {
                        Label(ru ? "Все действия…" : "All actions…", systemImage: "command")
                    }
                    Button {
                        appState.selectedTool = ru ? "Копировать" : "Copy"
                    } label: {
                        Label(ru ? "Копировать" : "Copy", systemImage: "plus.square.on.square")
                    }
                    Button {
                        appState.selectedTool = ru ? "Скрыть" : "Hide"
                    } label: {
                        Label(ru ? "Скрыть" : "Hide", systemImage: "eye.slash")
                    }
                } label: {
                    Image(systemName: "ellipsis")
                        .frame(width: 28, height: 28)
                }
                .menuStyle(.borderlessButton)
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .background(MirTheme.Colors.surfaceRaised.opacity(0.9), in: RoundedRectangle(cornerRadius: 7))

                Divider().frame(height: 22)

                actionButton("trash", ru ? "Удалить" : "Delete", destructive: true) {
                    appState.selectedTool = ru ? "Удалить" : "Delete"
                }
            }
            .padding(.horizontal, 9)
            .padding(.vertical, 6)
            .background(.ultraThinMaterial, in: Capsule())
            .overlay(Capsule().stroke(MirTheme.Colors.panelBorder.opacity(0.9), lineWidth: 1))
            .shadow(color: .black.opacity(0.22), radius: 12, y: 5)
            .transition(.opacity.combined(with: .move(edge: .bottom)))
            .animation(.easeOut(duration: 0.16), value: hasSelection)
        }
    }

    private var selectionBadge: some View {
        HStack(spacing: 6) {
            Image(systemName: badgeIcon)
                .foregroundStyle(MirTheme.Colors.accentBright)
            Text(badgeText)
                .font(.system(size: 11, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.textPrimary)
                .lineLimit(1)
                .truncationMode(.middle)
        }
        .frame(maxWidth: 200)
    }

    private var badgeIcon: String {
        switch appState.selection.primaryKind {
            case .vertex: return "circle.dotted"
            case .edge: return "skew"
            case .face: return "square.transparent"
            case .body: return count > 1 ? "square.stack.3d.up" : "cube.transparent"
            default: return "cube.transparent"
        }
    }

    private var badgeText: String {
        if count > 1 {
            return "\(appState.selection.primaryLabel) · \(count)"
        }
        return appState.selection.primaryLabel
    }

    private func commandButton(_ command: CADCommand) -> some View {
        Button {
            MirEventBus.shared.publish(.commandRequested(command.id))
            MirEventBus.shared.publish(.commandStarted(command.id))
            command.execute()
            MirEventBus.shared.publish(.commandFinished(command.id))
        } label: {
            Label(command.localizedTitle(appState.ui.language), systemImage: command.icon)
                .font(.system(size: 10, weight: .medium))
                .labelStyle(.titleAndIcon)
                .padding(.horizontal, 7)
                .frame(height: 28)
        }
        .buttonStyle(.plain)
        .foregroundStyle(MirTheme.Colors.textSecondary)
        .background(MirTheme.Colors.surfaceRaised.opacity(0.88), in: RoundedRectangle(cornerRadius: 7))
        .help(command.shortcut.map { "\(command.localizedTitle(appState.ui.language)) · \($0)" } ?? command.localizedTitle(appState.ui.language))
    }

    private func actionButton(_ icon: String, _ title: String, enabled: Bool = true, destructive: Bool = false, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Label(title, systemImage: icon)
                .font(.system(size: 10, weight: .medium))
                .labelStyle(.titleAndIcon)
                .padding(.horizontal, 7)
                .frame(height: 28)
        }
        .buttonStyle(.plain)
        .foregroundStyle(destructive ? .red : MirTheme.Colors.textSecondary)
        .background(MirTheme.Colors.surfaceRaised.opacity(enabled ? 0.88 : 0.45), in: RoundedRectangle(cornerRadius: 7))
        .opacity(enabled ? 1 : 0.45)
        .disabled(!enabled)
    }
}
