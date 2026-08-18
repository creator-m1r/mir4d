import SwiftUI

/// Compact floating action dock. It keeps utility actions available without
/// permanently consuming the lower viewport area.
struct MIR4DViewportActionDock: View {
    let onCommandPalette: () -> Void
    let onSettings: () -> Void
    let onPanels: () -> Void

    var body: some View {
        HStack(spacing: 6) {
            dockButton("command", "Команды", action: onCommandPalette)
            dockButton("slider.horizontal.3", "Настройки", action: onSettings)
            dockButton("rectangle.3.group", "Панели", action: onPanels)
        }
        .padding(6)
        .background(.ultraThinMaterial, in: Capsule())
        .overlay(Capsule().stroke(Color.white.opacity(0.10), lineWidth: 1))
        .shadow(color: .black.opacity(0.25), radius: 14, y: 6)
    }

    private func dockButton(_ systemName: String, _ label: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: systemName)
                .font(.system(size: 13, weight: .semibold))
                .frame(width: 34, height: 34)
                .contentShape(Circle())
        }
        .buttonStyle(.plain)
        .foregroundStyle(.white.opacity(0.78))
        .accessibilityLabel(label)
    }
}
