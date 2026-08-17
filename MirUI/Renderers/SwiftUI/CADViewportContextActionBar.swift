import SwiftUI

/// Contextual actions for the object currently selected in the CAD viewport.
/// The bar is intentionally transient: it appears only when the selection has
/// a meaningful target, keeping the viewport free from permanent tool chrome.
struct CADViewportContextActionBar: View {
    @ObservedObject var appState: CADAppState
    var onCommandPalette: (() -> Void)? = nil

    private var ru: Bool { appState.ui.language == .russian }
    private var hasSelection: Bool { appState.selection.hasSelection }
    private var count: Int { appState.selectionCount }

    var body: some View {
        if hasSelection {
            HStack(spacing: 6) {
                selectionBadge
                Divider().frame(height: 22)
                actionButton("pencil", ru ? "Изменить" : "Edit", enabled: count == 1) {
                    appState.selectedTool = ru ? "Изменить" : "Edit"
                }
                actionButton("move.3d", ru ? "Переместить" : "Move") {
                    appState.selectedTool = ru ? "Переместить" : "Move"
                }
                actionButton("ruler", ru ? "Измерить" : "Measure") {
                    appState.selectedTool = ru ? "Измерить" : "Measure"
                }
                Divider().frame(height: 22)
                Menu {
                    Button { appState.selectedTool = ru ? "Копировать" : "Copy" } label: {
                        Label(ru ? "Копировать" : "Copy", systemImage: "plus.square.on.square")
                    }
                    Button { appState.selectedTool = ru ? "Скрыть" : "Hide" } label: {
                        Label(ru ? "Скрыть" : "Hide", systemImage: "eye.slash")
                    }
                    Button { onCommandPalette?() } label: {
                        Label(ru ? "Все действия…" : "All actions…", systemImage: "command")
                    }
                } label: {
                    Image(systemName: "ellipsis").frame(width: 28, height: 28)
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
            Image(systemName: count > 1 ? "square.stack.3d.up" : "cube.transparent")
                .foregroundStyle(MirTheme.Colors.accentBright)
            Text(count > 1
                 ? "\(count) \(ru ? "объекта" : "objects")"
                 : (appState.selectedTreeItem.isEmpty ? (ru ? "Объект" : "Object") : appState.selectedTreeItem))
                .font(.system(size: 11, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.textPrimary)
                .lineLimit(1)
                .truncationMode(.middle)
        }
        .frame(maxWidth: 170)
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
