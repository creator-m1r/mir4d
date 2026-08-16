import SwiftUI

/// Renders a single dockable panel body.
/// UI-only presentation layer: MirEngine remains untouched.
struct CADPanelView: View {
    let panel: CADPanel
    @ObservedObject var appState: CADAppState
    @ObservedObject private var workspace = MIR4DWorkspaceCustomizationStore.shared

    var body: some View {
        VStack(spacing: 0) {
            if workspace.showPanelHeaders { panelHeader }
            panelContent
        }
        .background(MirTheme.Colors.panel.opacity(workspace.panelOpacity))
        .clipShape(RoundedRectangle(cornerRadius: workspace.panelCornerRadius))
        .overlay(alignment: .bottom) { Rectangle().fill(MirTheme.Colors.border).frame(height: 1) }
    }

    private var panelHeader: some View {
        HStack(spacing: workspace.compactPanels ? 6 : 9) {
            Image(systemName: panelIcon)
                .font(.system(size: workspace.compactPanels ? 10 : 11, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)
                .frame(width: workspace.compactPanels ? 20 : 22, height: workspace.compactPanels ? 20 : 22)
                .background(MirTheme.Colors.accentSoft)
                .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))

            VStack(alignment: .leading, spacing: 2) {
                Text(panelTitle).font(MirTheme.Typography.bodySemibold).foregroundStyle(MirTheme.Colors.textPrimary)
                if !workspace.compactPanels { Text(panelSubtitle).font(MirTheme.Typography.status).foregroundStyle(MirTheme.Colors.textTertiary) }
            }
            Spacer(minLength: 8)

            Menu {
                Button { appState.togglePanel(panel) } label: { Label(appState.ui.language == .russian ? "Скрыть панель" : "Hide panel", systemImage: "eye.slash") }
                Button { appState.setPanelPlacement(.left, for: panel) } label: { Label(appState.ui.language == .russian ? "Переместить влево" : "Move left", systemImage: "sidebar.leading") }
                Button { appState.setPanelPlacement(.right, for: panel) } label: { Label(appState.ui.language == .russian ? "Переместить вправо" : "Move right", systemImage: "sidebar.trailing") }
                Button { appState.setPanelPlacement(.bottom, for: panel) } label: { Label(appState.ui.language == .russian ? "Переместить вниз" : "Move bottom", systemImage: "rectangle.bottomhalf.inset.filled") }
                Button { appState.setPanelPlacement(.floating, for: panel) } label: { Label(appState.ui.language == .russian ? "Отделить в окно" : "Detach as window", systemImage: "macwindow.on.rectangle") }
            } label: {
                Image(systemName: "ellipsis").font(.system(size: 10, weight: .semibold)).frame(width: 24, height: 24)
            }
            .menuStyle(.borderlessButton)
            .foregroundStyle(MirTheme.Colors.textTertiary)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.85), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border, lineWidth: 1))

            Button { appState.togglePanel(panel) } label: {
                Image(systemName: "xmark").font(.system(size: 9, weight: .semibold)).frame(width: 24, height: 24)
            }
            .buttonStyle(.plain)
            .foregroundStyle(MirTheme.Colors.textTertiary)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.85), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .help(appState.ui.language == .russian ? "Скрыть панель" : "Hide panel")
        }
        .padding(.horizontal, workspace.compactPanels ? 8 : 11)
        .padding(.vertical, workspace.compactPanels ? 5 : 8)
        .background(MirTheme.Colors.surface.opacity(workspace.panelOpacity))
        .overlay(alignment: .bottom) { Rectangle().fill(MirTheme.Colors.border.opacity(0.8)).frame(height: 1) }
    }

    @ViewBuilder
    private var panelContent: some View {
        switch panel {
        case .project: projectPanel
        case .properties: propertiesPanel
        case .timeline: timeline
        case .simulation: simulationPanel
        case .history: historyPanel
        case .aiInspector: aiInspectorPanel
        }
    }

    private var projectPanel: some View {
        SidebarView(appState: appState)
    }

    private var propertiesPanel: some View {
        ScrollView {
            VStack(spacing: 0) {
                SelectionIdentityInspector(appState: appState)
                InspectorTabsView(appState: appState)
            }
            .frame(maxWidth: .infinity, alignment: .top)
        }
        .scrollIndicators(.hidden)
    }

    @ViewBuilder private var timeline: some View {
        switch appState.workbench {
        case .fourD, .simulation:
            FourDTimelineView(appState: appState).frame(minHeight: 170, idealHeight: 230, maxHeight: 300)
        case .assembly:
            TimelinePanelView(appState: appState).frame(minHeight: 170, idealHeight: 220, maxHeight: 280)
        default:
            TimelinePanelView(appState: appState).frame(minHeight: 190, idealHeight: 250, maxHeight: 320)
        }
    }

    private var simulationPanel: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 14) {
                panelSection(title: appState.ui.language == .russian ? "ФИЗИКА" : "PHYSICS") {
                    Picker(appState.ui.language == .russian ? "Тип" : "Type", selection: Binding(
                        get: { appState.simulation.physics },
                        set: { appState.simulation.physics = $0 }
                    )) {
                        ForEach(CADSimulationState.PhysicsType.allCases) { type in
                            Text(appState.ui.language == .russian ? type.titleRU : type.titleEN).tag(type)
                        }
                    }
                    .pickerStyle(.menu)
                }

                panelSection(title: appState.ui.language == .russian ? "СОСТОЯНИЕ РЕШАТЕЛЯ" : "SOLVER STATUS") {
                    HStack(spacing: 10) {
                        Circle().fill(appState.simulation.isRunning ? MirTheme.Colors.accent : .green).frame(width: 8, height: 8)
                        Text(appState.simulation.isRunning ? (appState.ui.language == .russian ? "Расчёт выполняется" : "Solving") : appState.simulation.solverStatus)
                            .font(.subheadline.weight(.medium))
                        Spacer()
                    }
                    ProgressView(value: appState.simulation.progress).tint(MirTheme.Colors.accent)
                }

                HStack(spacing: 8) {
                    simulationAction(appState.ui.language == .russian ? "Настроить" : "Setup", icon: "slider.horizontal.3") {
                        appState.simulation.phase = .setup
                    }
                    simulationAction(appState.ui.language == .russian ? "Расчёт" : "Solve", icon: "play.fill") {
                        appState.simulation.phase = .solve
                        appState.showNotification(appState.ui.language == .russian ? "Запуск расчёта передан решателю" : "Solve request sent to solver", type: .info)
                    }
                    simulationAction(appState.ui.language == .russian ? "Результаты" : "Results", icon: "chart.xyaxis.line") {
                        appState.simulation.phase = .results
                    }
                }
            }
            .padding(12)
        }
        .scrollIndicators(.hidden)
    }

    private var historyPanel: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 8) {
                historyRow(icon: "square.stack.3d.up", title: appState.ui.language == .russian ? "Текущее состояние модели" : "Current model state", detail: appState.selectedTreeItem)
                historyRow(icon: "cursorarrow.rays", title: appState.ui.language == .russian ? "Выбор объекта" : "Object selection", detail: "\(appState.selectionCount) selected")
                historyRow(icon: "hammer", title: appState.ui.language == .russian ? "Активный инструмент" : "Active tool", detail: appState.selectedTool)
                historyRow(icon: "rectangle.3.group", title: appState.ui.language == .russian ? "Рабочая среда" : "Workbench", detail: appState.ui.language == .russian ? appState.workbench.titleRU : appState.workbench.titleEN)
                historyRow(icon: "clock", title: appState.ui.language == .russian ? "Время" : "Time", detail: String(format: "%.2f", appState.currentTime))
            }
            .padding(12)
        }
        .scrollIndicators(.hidden)
    }

    private var aiInspectorPanel: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 12) {
                HStack(spacing: 9) {
                    Image(systemName: "sparkles").foregroundStyle(MirTheme.Colors.accentBright)
                    VStack(alignment: .leading, spacing: 2) {
                        Text("AI Inspector").font(.headline)
                        Text(appState.ui.language == .russian ? "Контекстный инженерный анализ" : "Contextual engineering analysis")
                            .font(.caption).foregroundStyle(.secondary)
                    }
                }

                panelSection(title: appState.ui.language == .russian ? "КОНТЕКСТ" : "CONTEXT") {
                    contextValue(appState.ui.language == .russian ? "Выбрано" : "Selection", "\(appState.selectionCount)")
                    contextValue(appState.ui.language == .russian ? "Инструмент" : "Tool", appState.selectedTool)
                    contextValue(appState.ui.language == .russian ? "Среда" : "Workbench", appState.ui.language == .russian ? appState.workbench.titleRU : appState.workbench.titleEN)
                }

                VStack(alignment: .leading, spacing: 8) {
                    Text(appState.ui.language == .russian ? "ПОДСКАЗКА" : "SUGGESTION")
                        .font(.caption.weight(.semibold)).foregroundStyle(.secondary)
                    Text(appState.selection.hasSelection
                         ? (appState.ui.language == .russian ? "Выбранный объект готов к проверке геометрии, параметров и зависимостей." : "The selected object is ready for geometry, parameter and dependency inspection.")
                         : (appState.ui.language == .russian ? "Выберите объект в сцене или дереве проекта, чтобы получить инженерный контекст." : "Select an object in the scene or project tree to get engineering context."))
                        .font(.subheadline).foregroundStyle(MirTheme.Colors.textSecondary)
                }
                .padding(12)
                .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))

                Button {
                    appState.showNotification(appState.ui.language == .russian ? "AI Inspector получил текущий контекст" : "AI Inspector received the current context", type: .success)
                } label: {
                    Label(appState.ui.language == .russian ? "Проверить контекст" : "Inspect context", systemImage: "wand.and.stars")
                        .frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .tint(MirTheme.Colors.accent)
            }
            .padding(12)
        }
        .scrollIndicators(.hidden)
    }

    private func panelSection<Content: View>(title: String, @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 8) {
            Text(title).font(.caption.weight(.semibold)).tracking(0.6).foregroundStyle(.secondary)
            content()
        }
    }

    private func contextValue(_ title: String, _ value: String) -> some View {
        HStack {
            Text(title).font(.caption).foregroundStyle(.secondary)
            Spacer()
            Text(value).font(.system(.caption, design: .monospaced)).foregroundStyle(MirTheme.Colors.textPrimary)
        }
    }

    private func simulationAction(_ title: String, icon: String, action: @escaping () -> Void) -> some View {
        Button(action: action) { Label(title, systemImage: icon).frame(maxWidth: .infinity) }
            .buttonStyle(.bordered).controlSize(.small)
    }

    private func historyRow(icon: String, title: String, detail: String) -> some View {
        HStack(spacing: 9) {
            Image(systemName: icon).frame(width: 24, height: 24).foregroundStyle(MirTheme.Colors.accentBright)
            VStack(alignment: .leading, spacing: 2) {
                Text(title).font(.subheadline.weight(.medium))
                Text(detail).font(.caption).foregroundStyle(.secondary).lineLimit(1)
            }
            Spacer()
        }
        .padding(9)
        .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
    }

    private var panelTitle: String {
        switch panel {
        case .project: return appState.ui.language == .russian ? "Навигатор" : "Navigator"
        case .properties: return appState.ui.language == .russian ? "Инспектор" : "Inspector"
        case .timeline: return appState.ui.language == .russian ? "Временная шкала" : "Timeline"
        case .simulation: return appState.ui.language == .russian ? "Симуляция" : "Simulation"
        case .history: return appState.ui.language == .russian ? "История" : "History"
        case .aiInspector: return "AI Inspector"
        }
    }

    private var panelSubtitle: String {
        switch panel {
        case .project: return appState.ui.language == .russian ? "Структура проекта" : "Project structure"
        case .properties: return appState.ui.language == .russian ? "Свойства выбранного объекта" : "Selected object properties"
        case .timeline: return appState.workbench == .fourD || appState.workbench == .simulation ? (appState.ui.language == .russian ? "4D и сценарии" : "4D and scenarios") : (appState.ui.language == .russian ? "Временная последовательность" : "Time sequence")
        case .simulation: return appState.ui.language == .russian ? "Расчётная постановка" : "Analysis setup"
        case .history: return appState.ui.language == .russian ? "Состояния интерфейса" : "Interface states"
        case .aiInspector: return appState.ui.language == .russian ? "Инженерный помощник" : "Engineering assistant"
        }
    }

    private var panelIcon: String {
        switch panel {
        case .project: return "list.bullet.indent"
        case .properties: return "slider.horizontal.3"
        case .timeline: return "timeline.selection"
        case .simulation: return "waveform.path.ecg"
        case .history: return "clock.arrow.circlepath"
        case .aiInspector: return "sparkles"
        }
    }
}
