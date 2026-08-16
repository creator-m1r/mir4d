import SwiftUI

/// Adaptive workbench ribbon for the MIR 4D command surface.
/// It exposes the current engineering context without duplicating engine state.
struct WorkbenchRibbonView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        VStack(spacing: 0) {
            workbenchTabs
            subModeStrip
        }
        .background(MirTheme.Colors.topBar)
        .overlay(alignment: .bottom) {
            Rectangle().fill(MirTheme.Colors.border).frame(height: 1)
        }
    }

    private var workbenchTabs: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 3) {
                ForEach(CADWorkbench.allCases) { workbench in
                    workbenchButton(workbench)
                }
                Spacer(minLength: 12)
            }
            .padding(.horizontal, MirTheme.Spacing.lg)
            .frame(height: 38)
        }
        .background(MirTheme.Colors.surface)
    }

    private func workbenchButton(_ workbench: CADWorkbench) -> some View {
        let active = appState.workbench == workbench
        return Button {
            withAnimation(.easeOut(duration: 0.16)) {
                appState.selectWorkbench(workbench)
            }
        } label: {
            HStack(spacing: 6) {
                Image(systemName: workbench.icon)
                    .font(.system(size: 10, weight: .semibold))
                Text(russian ? workbench.titleRU : workbench.titleEN)
                    .font(.system(size: 10, weight: active ? .semibold : .medium))
            }
            .foregroundStyle(active ? MirTheme.Colors.textPrimary : MirTheme.Colors.textSecondary)
            .padding(.horizontal, 10)
            .frame(height: 29)
            .background(active ? MirTheme.Colors.accentSoft : MirTheme.Colors.surfaceRaised.opacity(0.55), in: Capsule())
            .overlay {
                Capsule().stroke(active ? MirTheme.Colors.accent.opacity(0.7) : MirTheme.Colors.border.opacity(0.55), lineWidth: 1)
            }
        }
        .buttonStyle(.plain)
        .help(russian ? "Рабочая среда: \(workbench.titleRU)" : "Workbench: \(workbench.titleEN)")
    }

    private var subModeStrip: some View {
        HStack(spacing: 5) {
            HStack(spacing: 5) {
                Circle().fill(MirTheme.Colors.accentBright).frame(width: 5, height: 5)
                Text(russian ? "РЕЖИМ" : "MODE")
                    .font(.system(size: 8, weight: .bold, design: .monospaced))
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .padding(.horizontal, 7)

            ForEach(subModes, id: \.id) { mode in
                subModeButton(mode)
            }

            Spacer(minLength: 8)

            contextIndicator
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .frame(height: 34)
        .background(MirTheme.Colors.surfaceRaised.opacity(0.72))
    }

    private var subModes: [CADSubMode] {
        CADSubMode.allCases.filter { $0.workbench == appState.workbench }
    }

    private func subModeButton(_ mode: CADSubMode) -> some View {
        let active = appState.subMode == mode
        return Button {
            withAnimation(.easeOut(duration: 0.12)) {
                appState.switchSubMode(to: mode)
            }
        } label: {
            Text(russian ? mode.titleRU : mode.titleEN)
                .font(.system(size: 9, weight: active ? .semibold : .medium))
                .foregroundStyle(active ? MirTheme.Colors.textPrimary : MirTheme.Colors.textTertiary)
                .padding(.horizontal, 8)
                .frame(height: 24)
                .background(active ? MirTheme.Colors.surface : .clear, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
                .overlay {
                    RoundedRectangle(cornerRadius: MirTheme.Radius.small)
                        .stroke(active ? MirTheme.Colors.borderStrong : .clear, lineWidth: 1)
                }
        }
        .buttonStyle(.plain)
        .help(russian ? mode.titleRU : mode.titleEN)
    }

    private var contextIndicator: some View {
        HStack(spacing: 6) {
            Image(systemName: selectionIcon)
                .font(.system(size: 9, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)
            Text(selectionText)
                .font(.system(size: 9, weight: .medium))
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .lineLimit(1)
        }
        .padding(.horizontal, 8)
        .frame(height: 24)
        .background(MirTheme.Colors.surface, in: Capsule())
        .overlay(Capsule().stroke(MirTheme.Colors.border, lineWidth: 1))
        .help(russian ? "Текущий инженерный контекст" : "Current engineering context")
    }

    private var selectionIcon: String {
        appState.selectionCount > 0 ? "scope" : "cursorarrow"
    }

    private var selectionText: String {
        if appState.selectionCount > 0 {
            return russian ? "Выбрано: \(appState.selectionCount)" : "Selected: \(appState.selectionCount)"
        }
        return russian ? "Нет выбора" : "No selection"
    }
}
