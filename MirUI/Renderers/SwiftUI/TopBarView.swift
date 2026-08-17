import SwiftUI

/// Единая верхняя полоса MIR 4D: бренд и документ, режимы, команды, вид,
/// инструменты активной среды, панели и статус — в одной строке.
/// Ранее эта область состояла из пяти отдельных полос (шапка, тулбар,
/// вкладки среды, лента инструментов, строка вида); теперь это одна
/// интуитивная панель с явными группами.
struct TopBarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry
    @Binding var commandPalettePresented: Bool
    @ObservedObject private var appearance = MirUIAppearanceStore.shared
    @State private var interfaceCustomizationPresented = false

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        HStack(spacing: 7) {
            brand
            barDivider
            modeSegment
            subModeMenu
            barDivider
            commandGroup
            barDivider
            viewGroup
            barDivider
            toolGroupScroll
            Spacer(minLength: 6)
            contextStatus
            experienceButton
            MirUIAppearanceToolbar(appearance: appearance)
            panelsButton
            avatar
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .frame(height: 46)
        .background(MirTheme.Colors.topBar)
        .overlay(alignment: .bottom) { Rectangle().fill(MirTheme.Colors.borderStrong).frame(height: 1) }
        .sheet(isPresented: $interfaceCustomizationPresented) { InterfaceCustomizationView(appState: appState) }
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
        }
    }

    private var barDivider: some View {
        Divider().frame(height: 24).foregroundStyle(MirTheme.Colors.border)
    }

    // MARK: - Бренд и документ

    private var brand: some View {
        HStack(spacing: 7) {
            Image(systemName: "sun.max.fill")
                .foregroundStyle(MirTheme.Colors.keyframe)
                .font(.system(size: 15))
            VStack(alignment: .leading, spacing: 0) {
                Text("МИР 4D")
                    .font(.system(size: 11, weight: .bold))
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                HStack(spacing: 4) {
                    Circle()
                        .fill(appState.documentDirty ? MirTheme.Colors.warning : MirTheme.Colors.success)
                        .frame(width: 5, height: 5)
                    Text(appState.documentName + (appState.documentDirty ? " •" : ""))
                        .font(.system(size: 8))
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                        .lineLimit(1)
                }
            }
        }
        .frame(maxWidth: 150, alignment: .leading)
        .help(russian ? "Документ МИР 4D" : "MIR 4D document")
    }

    // MARK: - Режимы

    /// Быстрое переключение рабочей среды (иконки, активная подсвечена).
    private var modeSegment: some View {
        HStack(spacing: 2) {
            ForEach(CADWorkbench.allCases) { workbench in
                let active = appState.workbench == workbench
                Button {
                    withAnimation(.easeOut(duration: 0.16)) {
                        appState.selectWorkbench(workbench)
                    }
                } label: {
                    Image(systemName: workbench.icon)
                        .font(.system(size: 11, weight: .semibold))
                        .foregroundStyle(active ? MirTheme.Colors.textPrimary : MirTheme.Colors.textSecondary)
                        .frame(width: 27, height: 27)
                        .background(active ? MirTheme.Colors.accentSoft : .clear, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
                        .overlay {
                            RoundedRectangle(cornerRadius: MirTheme.Radius.small)
                                .stroke(active ? MirTheme.Colors.accent.opacity(0.6) : .clear, lineWidth: 1)
                        }
                }
                .buttonStyle(.plain)
                .help(russian ? workbench.titleRU : workbench.titleEN)
            }
        }
    }

    /// Подрежим активной среды (раскрывающееся меню).
    private var subModeMenu: some View {
        Menu {
            ForEach(subModes, id: \.id) { mode in
                Button {
                    appState.switchSubMode(to: mode)
                } label: {
                    Label(russian ? mode.titleRU : mode.titleEN, systemImage: mode == appState.subMode ? "checkmark" : "circle")
                }
            }
        } label: {
            HStack(spacing: 4) {
                Text(russian ? appState.subMode.titleRU : appState.subMode.titleEN)
                    .font(.system(size: 9, weight: .medium))
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                Image(systemName: "chevron.down")
                    .font(.system(size: 8, weight: .semibold))
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .padding(.horizontal, 8)
            .frame(height: 27)
            .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border, lineWidth: 1))
        }
        .menuStyle(.borderlessButton)
        .help(russian ? "Режим активной среды" : "Active sub-mode")
    }

    private var subModes: [CADSubMode] {
        CADSubMode.allCases.filter { $0.workbench == appState.workbench }
    }

    // MARK: - Команды

    private var commandGroup: some View {
        HStack(spacing: 2) {
            iconButton("arrow.uturn.backward", russian ? "Отменить" : "Undo") { execute("history.undo") }
            iconButton("arrow.uturn.forward", russian ? "Повторить" : "Redo") { execute("history.redo") }
            iconButton("doc.badge.plus", russian ? "Новый" : "New") { execute("document.new") }
            paletteButton
        }
    }

    private var paletteButton: some View {
        Button {
            commandPalettePresented = true
        } label: {
            HStack(spacing: 4) {
                Image(systemName: "magnifyingglass").font(.system(size: 10, weight: .semibold))
                Text("⌘K").font(.system(size: 8, weight: .semibold))
            }
            .foregroundStyle(MirTheme.Colors.textSecondary)
            .frame(width: 30, height: 27)
            .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain)
        .keyboardShortcut("k", modifiers: [.command])
        .help(russian ? "Палитра команд" : "Command palette")
    }

    private func iconButton(_ icon: String, _ label: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: icon)
                .font(.system(size: 11, weight: .medium))
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .frame(width: 27, height: 27)
                .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain)
        .help(label)
    }

    // MARK: - Вид

    private var viewGroup: some View {
        HStack(spacing: 2) {
            ForEach(["viewport.grid", "viewport.axes", "viewport.section"], id: \.self) { id in
                if let command = registry.commands.first(where: { $0.id == id }) {
                    let available = command.workbenches.contains(appState.workbench)
                        && command.isAvailable(appState.activeContext)
                    Button {
                        guard available else { return }
                        execute(id)
                    } label: {
                        Image(systemName: command.icon)
                            .font(.system(size: 11, weight: .medium))
                            .foregroundStyle(available ? MirTheme.Colors.textSecondary : MirTheme.Colors.textDisabled)
                            .frame(width: 27, height: 27)
                            .background(
                                available
                                    ? MirTheme.Colors.surfaceRaised.opacity(0.72)
                                    : MirTheme.Colors.surfaceRaised.opacity(0.3),
                                in: RoundedRectangle(cornerRadius: MirTheme.Radius.small)
                            )
                    }
                    .buttonStyle(.plain)
                    .disabled(!available)
                    .help(command.localizedTitle(appState.ui.language) + (command.shortcut.map { " · \($0)" } ?? ""))
                }
            }
        }
    }

    // MARK: - Инструменты активной среды

    /// Компактные группы инструментов текущей среды (только иконки,
    /// подписи — во всплывающих подсказках; активный инструмент подсвечен).
    private var toolGroupScroll: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 4) {
                ForEach(toolGroups) { group in
                    HStack(spacing: 2) {
                        ForEach(group.commandIDs, id: \.self) { id in
                            if let command = registry.commands.first(where: { $0.id == id }) {
                                toolButton(command)
                            }
                        }
                    }
                    .padding(.horizontal, 3)
                    .background(MirTheme.Colors.surfaceRaised.opacity(0.5), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
                    .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border.opacity(0.6), lineWidth: 1))
                }
            }
        }
        .frame(maxWidth: .infinity)
    }

    private func toolButton(_ command: CADCommand) -> some View {
        let available = command.workbenches.contains(appState.workbench)
            && command.isAvailable(appState.activeContext)
        let active = isToolActive(command)
        return Button {
            guard available else { return }
            MirEventBus.shared.publish(.commandRequested(command.id))
            MirEventBus.shared.publish(.commandStarted(command.id))
            command.execute()
            MirEventBus.shared.publish(.commandFinished(command.id))
        } label: {
            Image(systemName: command.icon)
                .font(.system(size: 11, weight: .medium))
                .foregroundStyle(available ? (active ? MirTheme.Colors.accentBright : MirTheme.Colors.textSecondary) : MirTheme.Colors.textDisabled)
                .frame(width: 27, height: 27)
                .background(active ? MirTheme.Colors.accentSoft : .clear, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
                .overlay {
                    RoundedRectangle(cornerRadius: MirTheme.Radius.small)
                        .stroke(active ? MirTheme.Colors.accent.opacity(0.55) : .clear, lineWidth: 1)
                }
        }
        .buttonStyle(.plain)
        .disabled(!available)
        .help(command.localizedTitle(appState.ui.language) + (command.shortcut.map { " · \($0)" } ?? ""))
    }

    private func isToolActive(_ command: CADCommand) -> Bool {
        switch command.id {
        case "viewport.select": return appState.selectedTool == "select"
        case "viewport.pan": return appState.selectedTool == "pan"
        case "viewport.zoom": return appState.selectedTool == "zoom"
        case "measure.distance": return appState.selectedTool == "measure"
        case "sketch.line": return appState.selectedTool == "line"
        case "sketch.rectangle": return appState.selectedTool == "rectangle"
        case "sketch.circle": return appState.selectedTool == "circle"
        default: return false
        }
    }

    private var toolGroups: [ToolGroup] {
        switch appState.workbench {
        case .model:
            return [
                ToolGroup("Создание", ["create.body", "model.extrude", "model.revolve"]),
                ToolGroup("Выбор", ["viewport.select", "viewport.pan", "viewport.fit"]),
                ToolGroup("Измерение", ["measure.distance"])
            ]
        case .sketch:
            return [
                ToolGroup("Геометрия", ["sketch.line", "sketch.rectangle", "sketch.circle"]),
                ToolGroup("Ограничения", ["sketch.constraint", "sketch.dimension"]),
                ToolGroup("Навигация", ["viewport.select", "viewport.pan", "viewport.zoom", "viewport.fit"])
            ]
        case .assembly:
            return [
                ToolGroup("Сборка", ["assembly.mate", "assembly.interference"]),
                ToolGroup("Навигация", ["viewport.select", "viewport.pan", "viewport.fit"])
            ]
        case .simulation:
            return [
                ToolGroup("Расчёт", ["simulation.solve", "simulation.results"]),
                ToolGroup("Навигация", ["viewport.select", "viewport.pan", "viewport.fit"])
            ]
        case .fourD:
            return [
                ToolGroup("Время", ["fourD.play", "fourD.branch"]),
                ToolGroup("Анализ", ["fourD.compare", "fourD.whatIf"]),
                ToolGroup("Навигация", ["viewport.select", "viewport.pan", "viewport.fit"])
            ]
        case .drawing:
            return [ToolGroup("Навигация", ["viewport.select", "viewport.pan", "viewport.fit"])]
        case .collaboration:
            return [ToolGroup("Ревью", ["viewport.select", "viewport.pan", "viewport.fit"])]
        case .visualization:
            return [ToolGroup("Сцена", ["viewport.select", "viewport.pan", "viewport.fit"])]
        }
    }

    // MARK: - Статус и панели

    private var contextStatus: some View {
        HStack(spacing: 5) {
            Image(systemName: appState.selectionCount > 0 ? "scope" : "cursorarrow")
                .font(.system(size: 9, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)
            Text(appState.selectionCount > 0
                 ? (russian ? "Выбрано: \(appState.selectionCount)" : "Selected: \(appState.selectionCount)")
                 : (russian ? "Нет выбора" : "No selection"))
                .font(.system(size: 9, weight: .medium))
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .lineLimit(1)
        }
        .padding(.horizontal, 8)
        .frame(height: 27)
        .background(MirTheme.Colors.surface, in: Capsule())
        .overlay(Capsule().stroke(MirTheme.Colors.border, lineWidth: 1))
        .help(russian ? "Текущий инженерный контекст" : "Current engineering context")
    }

    private var experienceButton: some View {
        Button {
            appState.toggleExperience()
        } label: {
            HStack(spacing: 4) {
                Image(systemName: "slider.horizontal.3")
                Text(appState.ui.experience == .expert ? "EXP" : "BAS")
            }
            .font(.system(size: 9, weight: .bold, design: .monospaced))
            .foregroundStyle(MirTheme.Colors.textSecondary)
            .padding(.horizontal, 7)
            .frame(height: 27)
            .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain)
        .help(appState.ui.experience == .expert ? (russian ? "Экспертный режим" : "Expert mode") : (russian ? "Базовый режим" : "Basic mode"))
    }

    private var panelsButton: some View {
        Button {
            interfaceCustomizationPresented = true
        } label: {
            Image(systemName: "rectangle.3.group")
                .font(.system(size: 11, weight: .semibold))
                .frame(width: 27, height: 27)
        }
        .buttonStyle(.plain)
        .foregroundStyle(MirTheme.Colors.textSecondary)
        .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        .help(russian ? "Панели: состав и расположение" : "Panels: layout and visibility")
    }

    private var avatar: some View {
        Circle()
            .fill(LinearGradient(colors: [MirTheme.Colors.accent, MirTheme.Colors.accentBright], startPoint: .topLeading, endPoint: .bottomTrailing))
            .frame(width: 27, height: 27)
            .overlay { Text("M1R").font(.system(size: 7, weight: .bold)).foregroundStyle(.white) }
            .help("МИР 4D")
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
}

private struct ToolGroup: Identifiable {
    let id = UUID()
    let title: String
    let commandIDs: [String]

    init(_ title: String, _ commandIDs: [String]) {
        self.title = title
        self.commandIDs = commandIDs
    }
}