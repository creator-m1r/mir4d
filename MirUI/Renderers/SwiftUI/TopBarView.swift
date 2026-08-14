import SwiftUI

struct TopBarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry
    @Binding var commandPalettePresented: Bool
    @ObservedObject private var appearance = MirUIAppearanceStore.shared

    var body: some View {
        VStack(spacing: 0) {
            mainBar
            ContextualCommandBarView(appState: appState, registry: registry)
        }
        .background(.ultraThinMaterial)
        .overlay(alignment: .bottom) {
            Rectangle().fill(MirTheme.Colors.border).frame(height: 1)
        }
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
        }
    }

    private var mainBar: some View {
        HStack(spacing: MirTheme.Spacing.md) {
            brand
            Divider().frame(height: 24)
            WorkbenchSwitcher(appState: appState)
            Text(subModeTitle)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .padding(.horizontal, 7).padding(.vertical, 4)
                .background(Color.white.opacity(0.035)).clipShape(Capsule())
            Spacer()
            commandPaletteButton
            historyButtons
            utilityButtons
            Divider().frame(height: 24)
            Button { appState.toggleExperience() } label: {
                Text(appState.ui.experience == .expert ? "EXP" : "BAS")
                    .font(.system(size: 9, weight: .bold, design: .monospaced))
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                    .frame(width: 34, height: 28)
                    .background(Color.white.opacity(0.04))
                    .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            }
            .buttonStyle(.plain)
            .help(appState.ui.experience == .expert ? "Expert" : "Beginner")

            MirUIAppearanceToolbar(appearance: appearance)

            Circle()
                .fill(LinearGradient(colors: [MirTheme.Colors.accent, MirTheme.Colors.accentBright], startPoint: .topLeading, endPoint: .bottomTrailing))
                .frame(width: 30, height: 30)
                .overlay { Text("M1R").font(.system(size: 8, weight: .bold)).foregroundStyle(.white) }
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .frame(height: 54)
    }

    private var brand: some View {
        HStack(spacing: MirTheme.Spacing.sm) {
            Image(systemName: "sun.max.fill").foregroundStyle(MirTheme.Colors.keyframe).font(.system(size: 17))
            VStack(alignment: .leading, spacing: 1) {
                Text("M1R.PRO · 4D CAD").font(.system(size: 13, weight: .bold)).foregroundStyle(MirTheme.Colors.textPrimary)
                HStack(spacing: 5) {
                    Circle().fill(MirTheme.Colors.success).frame(width: 6, height: 6)
                    Text(appState.documentName + (appState.documentDirty ? " •" : ""))
                        .font(MirTheme.Typography.status).foregroundStyle(MirTheme.Colors.textTertiary)
                }
            }
        }
    }

    private var commandPaletteButton: some View {
        Button { commandPalettePresented = true } label: {
            HStack(spacing: 6) {
                Image(systemName: "magnifyingglass")
                Text(appState.ui.language == .russian ? "Команды" : "Commands")
                Text("⌘K").foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .font(MirTheme.Typography.caption).foregroundStyle(MirTheme.Colors.textSecondary)
            .padding(.horizontal, 10).padding(.vertical, 7)
            .background(Color.white.opacity(0.04)).clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        }
        .buttonStyle(.plain).keyboardShortcut("k", modifiers: [.command])
    }

    private var historyButtons: some View {
        HStack(spacing: 2) {
            topButton("arrow.uturn.backward", "⌘Z") { execute("history.undo") }
            topButton("arrow.uturn.forward", "⇧⌘Z") { execute("history.redo") }
        }
    }

    private var utilityButtons: some View {
        HStack(spacing: 4) {
            topButton("doc.badge.plus", appState.ui.language == .russian ? "Новый" : "New") { execute("document.new") }
            topButton("square.grid.2x2", appState.ui.language == .russian ? "Панели" : "Panels") {
                appState.togglePanel(.project); appState.togglePanel(.properties)
            }
            topButton("sparkles", "AI") { appState.togglePanel(.aiInspector) }
        }
    }

    private func topButton(_ icon: String, _ label: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: icon).font(.system(size: 12)).frame(width: 28, height: 28)
        }
        .buttonStyle(.plain).foregroundStyle(MirTheme.Colors.textSecondary)
        .background(Color.white.opacity(0.035)).clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        .help(label)
    }

    private func execute(_ id: String) {
        guard let command = registry.commands.first(where: { $0.id == id }) else {
            MirEventBus.shared.publish(.commandRequested(id)); return
        }
        guard command.workbenches.contains(appState.workbench), command.isAvailable(appState.activeContext) else { return }
        MirEventBus.shared.publish(.commandRequested(id)); MirEventBus.shared.publish(.commandStarted(id))
        command.execute(); MirEventBus.shared.publish(.commandFinished(id))
    }

    private var subModeTitle: String { appState.ui.language == .russian ? appState.subMode.titleRU : appState.subMode.titleEN }
}
