import SwiftUI

struct TopBarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry
    @Binding var commandPalettePresented: Bool
    @ObservedObject private var appearance = MirUIAppearanceStore.shared
    @State private var interfaceCustomizationPresented = false

    var body: some View {
        VStack(spacing: 0) {
            applicationHeader
            mainToolbar
            ContextualCommandBarView(appState: appState, registry: registry)
        }
        .background(MirTheme.Colors.surface)
        .overlay(alignment: .bottom) {
            Rectangle()
                .fill(MirTheme.Colors.borderStrong)
                .frame(height: 1)
        }
        .sheet(isPresented: $interfaceCustomizationPresented) {
            InterfaceCustomizationView(appState: appState)
        }
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
        }
    }

    /// The application header is an in-window view, not an overlay on top of
    /// the macOS title area. It is deliberately separated from the toolbar.
    private var applicationHeader: some View {
        HStack(spacing: 0) {
            brand
                .padding(.trailing, MirTheme.Spacing.lg)

            Divider()
                .frame(height: 22)
                .foregroundStyle(MirTheme.Colors.border)

            menuButton("Проект", icon: "folder") {
                execute("document.new")
            }
            menuButton("Правка", icon: "pencil") {
                commandPalettePresented = true
            }
            menuButton("Вид", icon: "eye") {
                appState.toggleGrid()
            }
            menuButton("Создание", icon: "plus.circle") {
                _ = registry.execute(id: "create.body", context: appState.activeContext)
            }
            menuButton("Инструменты", icon: "wrench.and.screwdriver") {
                commandPalettePresented = true
            }

            Spacer(minLength: MirTheme.Spacing.lg)

            Text(appState.ui.language == .russian ? "Рабочая среда" : "Workbench")
                .font(MirTheme.Typography.status)
                .foregroundStyle(MirTheme.Colors.textTertiary)

            Text(appState.ui.language == .russian ? appState.workbench.titleRU : appState.workbench.titleEN)
                .font(MirTheme.Typography.bodySemibold)
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
                .background(MirTheme.Colors.surfaceRaised, in: Capsule())
                .padding(.leading, 7)
                .padding(.trailing, MirTheme.Spacing.md)

            Button {
                interfaceCustomizationPresented = true
            } label: {
                Label(
                    appState.ui.language == .russian ? "Настроить интерфейс" : "Customize Interface",
                    systemImage: "rectangle.3.group"
                )
            }
            .buttonStyle(.bordered)
            .controlSize(.small)
            .help(appState.ui.language == .russian ? "Открыть отдельный режим настройки панелей" : "Open the dedicated panel customization mode")
            .padding(.trailing, MirTheme.Spacing.lg)
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .frame(height: 38)
        .background(MirTheme.Colors.background)
    }

    private var mainToolbar: some View {
        HStack(spacing: MirTheme.Spacing.md) {
            WorkbenchSwitcher(appState: appState)

            Text(subModeTitle)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .padding(.horizontal, 8)
                .padding(.vertical, 5)
                .background(MirTheme.Colors.surfaceRaised, in: Capsule())

            Divider().frame(height: 24)

            commandPaletteButton
            historyButtons
            utilityButtons

            Spacer(minLength: MirTheme.Spacing.md)

            Button { appState.toggleExperience() } label: {
                Text(appState.ui.experience == .expert ? "EXP" : "BAS")
                    .font(.system(size: 9, weight: .bold, design: .monospaced))
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                    .frame(width: 38, height: 28)
                    .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            }
            .buttonStyle(.plain)
            .help(appState.ui.experience == .expert ? "Экспертный режим" : "Базовый режим")

            MirUIAppearanceToolbar(appearance: appearance)

            Circle()
                .fill(LinearGradient(colors: [MirTheme.Colors.accent, MirTheme.Colors.accentBright], startPoint: .topLeading, endPoint: .bottomTrailing))
                .frame(width: 30, height: 30)
                .overlay {
                    Text("M1R")
                        .font(.system(size: 8, weight: .bold))
                        .foregroundStyle(.white)
                }
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .frame(height: 48)
        .background(MirTheme.Colors.surface)
    }

    private var brand: some View {
        HStack(spacing: MirTheme.Spacing.sm) {
            Image(systemName: "sun.max.fill")
                .foregroundStyle(MirTheme.Colors.keyframe)
                .font(.system(size: 17))

            VStack(alignment: .leading, spacing: 1) {
                Text("МИР 4D")
                    .font(.system(size: 13, weight: .bold))
                    .foregroundStyle(MirTheme.Colors.textPrimary)

                HStack(spacing: 5) {
                    Circle()
                        .fill(MirTheme.Colors.success)
                        .frame(width: 6, height: 6)
                    Text(appState.documentName + (appState.documentDirty ? " •" : ""))
                        .font(MirTheme.Typography.status)
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                        .lineLimit(1)
                }
            }
        }
    }

    private func menuButton(_ title: String, icon: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Label(title, systemImage: icon)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .padding(.horizontal, 9)
                .frame(height: 28)
        }
        .buttonStyle(.plain)
        .contentShape(Rectangle())
    }

    private var commandPaletteButton: some View {
        Button { commandPalettePresented = true } label: {
            HStack(spacing: 6) {
                Image(systemName: "magnifyingglass")
                Text(appState.ui.language == .russian ? "Команды" : "Commands")
                Text("⌘K").foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .font(MirTheme.Typography.caption)
            .foregroundStyle(MirTheme.Colors.textSecondary)
            .padding(.horizontal, 10)
            .padding(.vertical, 7)
            .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        }
        .buttonStyle(.plain)
        .keyboardShortcut("k", modifiers: [.command])
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
                interfaceCustomizationPresented = true
            }
            topButton("sparkles", "AI") { appState.togglePanel(.aiInspector) }
        }
    }

    private func topButton(_ icon: String, _ label: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: icon)
                .font(.system(size: 12, weight: .medium))
                .frame(width: 30, height: 28)
        }
        .buttonStyle(.plain)
        .foregroundStyle(MirTheme.Colors.textSecondary)
        .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        .help(label)
    }

    private func execute(_ id: String) {
        guard let command = registry.commands.first(where: { $0.id == id }) else {
            MirEventBus.shared.publish(.commandRequested(id))
            return
        }
        guard command.workbenches.contains(appState.workbench), command.isAvailable(appState.activeContext) else { return }
        MirEventBus.shared.publish(.commandRequested(id))
        MirEventBus.shared.publish(.commandStarted(id))
        command.execute()
        MirEventBus.shared.publish(.commandFinished(id))
    }

    private var subModeTitle: String {
        appState.ui.language == .russian ? appState.subMode.titleRU : appState.subMode.titleEN
    }
}
