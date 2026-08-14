import SwiftUI

struct TopBarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry
    @Binding var commandPalettePresented: Bool
    @ObservedObject private var appearance = MirUIAppearanceStore.shared
    @State private var interfaceCustomizationPresented = false

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        VStack(spacing: 0) {
            applicationHeader
            mainToolbar
            ContextualCommandBarView(appState: appState, registry: registry)
        }
        .background(MirTheme.Colors.background)
        .overlay(alignment: .bottom) {
            Rectangle().fill(MirTheme.Colors.borderStrong).frame(height: 1)
        }
        .sheet(isPresented: $interfaceCustomizationPresented) {
            InterfaceCustomizationView(appState: appState)
        }
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
        }
    }

    /// Собственная непрозрачная полоса приложения является частью SwiftUI-окна.
    /// Она не перекрывает системную область заголовка macOS.
    private var applicationHeader: some View {
        HStack(spacing: 0) {
            brand.padding(.trailing, MirTheme.Spacing.lg)
            Divider().frame(height: 22).foregroundStyle(MirTheme.Colors.border)

            applicationMenu(russian ? "Проект" : "Project", icon: "folder") {
                Button(russian ? "Новый проект" : "New Project") { execute("document.new") }
                Divider()
                Button(russian ? "Открыть проект…" : "Open Project…") { execute("document.open") }
                Button(russian ? "Сохранить" : "Save") { execute("document.save") }
            }
            applicationMenu(russian ? "Правка" : "Edit", icon: "pencil") {
                Button(russian ? "Отменить" : "Undo") { execute("history.undo") }
                Button(russian ? "Повторить" : "Redo") { execute("history.redo") }
                Divider()
                Button(russian ? "Палитра команд…" : "Command Palette…") { commandPalettePresented = true }
            }
            applicationMenu(russian ? "Вид" : "View", icon: "eye") {
                Button(russian ? "Сетка" : "Grid") { appState.toggleGrid() }
                Button(russian ? "Оси" : "Axes") { appState.toggleAxes() }
                Divider()
                Button(russian ? "Настроить интерфейс…" : "Customize Interface…") { interfaceCustomizationPresented = true }
            }
            applicationMenu(russian ? "Создание" : "Create", icon: "plus.circle") {
                Button(russian ? "Эскиз" : "Sketch") { execute("create.sketch") }
                Button(russian ? "Тело" : "Body") { execute("create.body") }
            }
            applicationMenu(russian ? "Инструменты" : "Tools", icon: "wrench.and.screwdriver") {
                Button(russian ? "Измерить" : "Measure") { execute("inspect.measure") }
                Button(russian ? "Палитра команд…" : "Command Palette…") { commandPalettePresented = true }
            }

            Spacer(minLength: MirTheme.Spacing.lg)

            HStack(spacing: 7) {
                Circle().fill(MirTheme.Colors.success).frame(width: 6, height: 6)
                Text(russian ? "Рабочая среда" : "Workbench")
                    .font(MirTheme.Typography.status).foregroundStyle(MirTheme.Colors.textTertiary)
                Text(russian ? appState.workbench.titleRU : appState.workbench.titleEN)
                    .font(MirTheme.Typography.bodySemibold).foregroundStyle(MirTheme.Colors.textPrimary)
            }
            .padding(.horizontal, 10).padding(.vertical, 5)
            .background(MirTheme.Colors.surfaceRaised, in: Capsule())
            .help(russian ? "Текущая рабочая среда" : "Current workbench")

            Button { interfaceCustomizationPresented = true } label: {
                Image(systemName: "rectangle.3.group")
                    .font(.system(size: 12, weight: .semibold)).frame(width: 30, height: 28)
            }
            .buttonStyle(.plain).foregroundStyle(MirTheme.Colors.textSecondary)
            .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .help(russian ? "Настроить панели интерфейса" : "Customize interface panels")
            .padding(.leading, 8).padding(.trailing, MirTheme.Spacing.md)
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .frame(height: 42)
        // ОПАКОВАННЫЙ, НЕПРОЗРАЧНЫЙ фон: верхняя полоса не просвечивает содержимое окна.
        .background(MirTheme.Colors.background)
        .overlay(alignment: .bottom) {
            Rectangle().fill(MirTheme.Colors.border).frame(height: 1)
        }
    }

    private func applicationMenu(_ title: String, icon: String, @ViewBuilder content: () -> some View) -> some View {
        Menu { content() } label: {
            Label(title, systemImage: icon).font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary).padding(.horizontal, 9).frame(height: 30)
        }
        .menuStyle(.borderlessButton).contentShape(Rectangle())
    }

    private var mainToolbar: some View {
        HStack(spacing: MirTheme.Spacing.sm) {
            WorkbenchSwitcher(appState: appState)
            Divider().frame(height: 26)
            commandPaletteButton
            historyButtons
            Divider().frame(height: 26)
            toolbarAction("doc.badge.plus", russian ? "Новый" : "New", "document.new")
            toolbarAction("pencil.and.ruler", russian ? "Эскиз" : "Sketch", "create.sketch")
            toolbarAction("cube.transparent", russian ? "Тело" : "Body", "create.body")
            toolbarAction("ruler", russian ? "Измерить" : "Measure", "inspect.measure")
            Spacer(minLength: MirTheme.Spacing.md)
            Button { appState.toggleExperience() } label: {
                HStack(spacing: 5) { Image(systemName: "slider.horizontal.3"); Text(appState.ui.experience == .expert ? "EXP" : "BAS") }
                    .font(.system(size: 9, weight: .bold, design: .monospaced)).foregroundStyle(MirTheme.Colors.textSecondary)
                    .padding(.horizontal, 8).frame(height: 28).background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            }.buttonStyle(.plain)
            .help(appState.ui.experience == .expert ? (russian ? "Экспертный режим" : "Expert mode") : (russian ? "Базовый режим" : "Basic mode"))
            MirUIAppearanceToolbar(appearance: appearance)
            Circle().fill(LinearGradient(colors: [MirTheme.Colors.accent, MirTheme.Colors.accentBright], startPoint: .topLeading, endPoint: .bottomTrailing))
                .frame(width: 30, height: 30).overlay { Text("M1R").font(.system(size: 8, weight: .bold)).foregroundStyle(.white) }.help("МИР 4D")
        }
        .padding(.horizontal, MirTheme.Spacing.lg).frame(height: 46)
        .background(MirTheme.Colors.surface)
    }

    private var brand: some View {
        HStack(spacing: MirTheme.Spacing.sm) {
            Image(systemName: "sun.max.fill").foregroundStyle(MirTheme.Colors.keyframe).font(.system(size: 17))
            VStack(alignment: .leading, spacing: 1) {
                Text("МИР 4D").font(.system(size: 13, weight: .bold)).foregroundStyle(MirTheme.Colors.textPrimary)
                HStack(spacing: 5) {
                    Circle().fill(appState.documentDirty ? MirTheme.Colors.warning : MirTheme.Colors.success).frame(width: 6, height: 6)
                    Text(appState.documentName + (appState.documentDirty ? " •" : "")).font(MirTheme.Typography.status).foregroundStyle(MirTheme.Colors.textTertiary).lineLimit(1)
                }
            }
        }.help(russian ? "Документ МИР 4D" : "MIR 4D document")
    }

    private var commandPaletteButton: some View {
        Button { commandPalettePresented = true } label: {
            HStack(spacing: 6) { Image(systemName: "magnifyingglass"); Text(russian ? "Команды" : "Commands"); Text("⌘K").foregroundStyle(MirTheme.Colors.textTertiary) }
                .font(MirTheme.Typography.caption).foregroundStyle(MirTheme.Colors.textSecondary)
                .padding(.horizontal, 10).padding(.vertical, 7).background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        }.buttonStyle(.plain).keyboardShortcut("k", modifiers: [.command]).help(russian ? "Палитра команд" : "Command palette")
    }

    private var historyButtons: some View {
        HStack(spacing: 2) {
            topButton("arrow.uturn.backward", russian ? "Отменить" : "Undo") { execute("history.undo") }
            topButton("arrow.uturn.forward", russian ? "Повторить" : "Redo") { execute("history.redo") }
        }
    }

    private func toolbarAction(_ icon: String, _ label: String, _ command: String) -> some View {
        Button { execute(command) } label: {
            HStack(spacing: 5) { Image(systemName: icon); Text(label) }.font(MirTheme.Typography.caption).foregroundStyle(MirTheme.Colors.textSecondary)
                .padding(.horizontal, 9).frame(height: 30).background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }.buttonStyle(.plain).help(label)
    }

    private func topButton(_ icon: String, _ label: String, action: @escaping () -> Void) -> some View {
        Button(action: action) { Image(systemName: icon).font(.system(size: 12, weight: .medium)).frame(width: 30, height: 28) }
            .buttonStyle(.plain).foregroundStyle(MirTheme.Colors.textSecondary)
            .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small)).help(label)
    }

    private func execute(_ id: String) {
        guard let command = registry.commands.first(where: { $0.id == id }) else { MirEventBus.shared.publish(.commandRequested(id)); return }
        guard command.workbenches.contains(appState.workbench), command.isAvailable(appState.activeContext) else { return }
        MirEventBus.shared.publish(.commandRequested(id)); MirEventBus.shared.publish(.commandStarted(id)); command.execute(); MirEventBus.shared.publish(.commandFinished(id))
    }
}
