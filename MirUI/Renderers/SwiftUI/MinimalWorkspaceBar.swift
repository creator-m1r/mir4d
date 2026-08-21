import SwiftUI

@MainActor
struct MinimalWorkspaceBar: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var workspace = MIR4DWorkspaceCustomizationStore.shared

    var onCommandPalette: () -> Void = {}
    var onSettings: () -> Void = {}
    var onPanels: () -> Void = {}

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        HStack(spacing: 6) {
            Button(action: onCommandPalette) {
                Label(russian ? "Палитра команд" : "Command palette", systemImage: "command")
            }
            .buttonStyle(.bordered)
            .help(russian ? "Палитра команд (⌘K)" : "Command palette (⌘K)")

            Button(action: onSettings) {
                Label(russian ? "Настройки" : "Settings", systemImage: "slider.horizontal.3")
            }
            .buttonStyle(.bordered)
            .help(russian ? "Настройки радиального меню" : "Radial menu settings")

            Button(action: onPanels) {
                Image(systemName: workspace.isMinimalMode ? "rectangle.split.3x1.fill" : "rectangle.3.group")
            }
            .buttonStyle(.bordered)
            .help(russian ? "Показать/скрыть панели" : "Show/hide panels")
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 6)
        .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 12))
        .overlay(
            RoundedRectangle(cornerRadius: 12)
                .stroke(MirTheme.Colors.panelBorder.opacity(0.55), lineWidth: 1)
        )
    }
}