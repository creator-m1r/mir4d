import SwiftUI

struct InspectorTabsView: View {
    @ObservedObject var appState: CADAppState
    @State private var search = ""
    @State private var activeTab: Tab = .parameters

    enum Tab: String, CaseIterable, Identifiable {
        case geometry
        case parameters
        case constraints
        case dimensions
        case material
        case simulationFields
        case contours
        case sensors
        case time
        case events
        case branches
        case history

        var id: String { rawValue }

        func title(_ language: CADLanguage) -> String {
            switch self {
            case .geometry: return language == .russian ? "Геометрия" : "Geometry"
            case .parameters: return language == .russian ? "Параметры" : "Parameters"
            case .constraints: return language == .russian ? "Ограничения" : "Constraints"
            case .dimensions: return language == .russian ? "Размеры" : "Dimensions"
            case .material: return language == .russian ? "Материал" : "Material"
            case .simulationFields: return language == .russian ? "Поля" : "Fields"
            case .contours: return language == .russian ? "Контуры" : "Contours"
            case .sensors: return language == .russian ? "Датчики" : "Sensors"
            case .time: return language == .russian ? "Время" : "Time"
            case .events: return language == .russian ? "События" : "Events"
            case .branches: return language == .russian ? "Ветки" : "Branches"
            case .history: return language == .russian ? "История" : "History"
            }
        }
    }

    private var context: CADActiveContext { appState.activeContext }

    private var tabs: [Tab] {
        switch context.workbench {
        case .sketch:
            switch context.subMode {
            case .sketchConstraint:
                return [.constraints, .geometry, .parameters, .history]
            case .sketchDimension:
                return [.dimensions, .constraints, .geometry, .parameters, .history]
            default:
                return [.geometry, .parameters, .constraints, .dimensions, .history]
            }

        case .model:
            return [.parameters, .geometry, .material, .history]

        case .assembly:
            return [.geometry, .parameters, .material, .history]

        case .simulation:
            switch context.simulation.phase {
            case .results:
                return [.simulationFields, .contours, .sensors, .parameters, .history]
            case .compare:
                return [.simulationFields, .contours, .sensors, .history]
            default:
                return [.parameters, .geometry, .material, .history]
            }

        case .fourD:
            return [.time, .events, .branches, .geometry, .parameters, .history]

        case .drawing:
            return [.geometry, .parameters, .history]

        case .collaboration:
            return [.history]

        case .visualization:
            return [.geometry, .parameters, .material, .history]
        }
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            header
            searchField
            tabBar
            Divider().opacity(0.25)
            content
        }
        .mirPanel()
        .onAppear(perform: ensureValidTab)
        .onChange(of: appState.workbench) { _, _ in ensureValidTab() }
        .onChange(of: appState.subMode) { _, _ in ensureValidTab() }
        .onChange(of: appState.simulation.phase) { _, _ in ensureValidTab() }
        .onChange(of: appState.ui.language) { _, _ in ensureValidTab() }
    }

    private var header: some View {
        HStack(spacing: 10) {
            Image(systemName: iconForSelection)
                .foregroundStyle(MirTheme.Colors.accentBright)

            VStack(alignment: .leading, spacing: 2) {
                Text(contextTitle)
                    .font(MirTheme.Typography.title)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                    .lineLimit(1)

                Text(selectionSubtitle)
                    .font(MirTheme.Typography.caption)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }

            Spacer()

            Text(localizedWorkbench)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
                .background(MirTheme.Colors.accentSoft)
                .clipShape(Capsule())
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .padding(.top, MirTheme.Spacing.lg)
        .padding(.bottom, MirTheme.Spacing.md)
    }

    private var searchField: some View {
        HStack(spacing: 6) {
            Image(systemName: "magnifyingglass")
                .font(.system(size: 10))
                .foregroundStyle(MirTheme.Colors.textTertiary)

            TextField(
                appState.ui.language == .russian ? "Поиск свойств…" : "Search properties…",
                text: $search
            )
            .textFieldStyle(.plain)
            .font(MirTheme.Typography.caption)
        }
        .padding(8)
        .background(MirTheme.Colors.border.opacity(0.35))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .padding(.horizontal, MirTheme.Spacing.lg)
        .padding(.bottom, MirTheme.Spacing.md)
    }

    private var tabBar: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 2) {
                ForEach(tabs) { tab in
                    Button {
                        activeTab = tab
                    } label: {
                        Text(tab.title(appState.ui.language))
                            .font(MirTheme.Typography.caption)
                            .foregroundStyle(activeTab == tab ? MirTheme.Colors.accentBright : MirTheme.Colors.textTertiary)
                            .padding(.horizontal, 8)
                            .padding(.vertical, 5)
                            .background(activeTab == tab ? MirTheme.Colors.accentSoft : Color.clear)
                            .clipShape(RoundedRectangle(cornerRadius: 5))
                    }
                    .buttonStyle(.plain)
                }
            }
            .padding(.horizontal, MirTheme.Spacing.lg)
            .padding(.bottom, MirTheme.Spacing.sm)
        }
    }

    @ViewBuilder
    private var content: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: MirTheme.Spacing.md) {
                switch activeTab {
                case .geometry: geometryContent
                case .parameters: parametersContent
                case .constraints: constraintsContent
                case .dimensions: dimensionsContent
                case .material: materialContent
                case .simulationFields: simulationFieldsContent
                case .contours: contoursContent
                case .sensors: sensorsContent
                case .time: timeContent
                case .events: eventsContent
                case .branches: branchesContent
                case .history: historyContent
                }
            }
            .padding(MirTheme.Spacing.lg)
        }
    }

    private var geometryContent: some View {
        InspectorGroup(title: localized("Контекст", "Context")) {
            InspectorRow(label: localized("Среда", "Workbench"), value: localizedWorkbench)
            InspectorRow(label: localized("Подрежим", "SubMode"), value: localizedSubMode)
            InspectorRow(label: localized("Выбрано", "Selected"), value: "\(context.selection.count)")
            InspectorRow(label: localized("Тип", "Type"), value: selectionKindTitle)
        }
    }

    private var parametersContent: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.md) {
            InspectorGroup(title: localized("Параметры контекста", "Context Parameters")) {
                InspectorRow(label: localized("Инструмент", "Tool"), value: appState.selectedTool)
                InspectorRow(label: localized("Привязка", "Snap"), value: appState.interaction.snapEnabled ? "ON" : "OFF")
                InspectorRow(label: localized("Вывод", "Inference"), value: appState.interaction.inferenceEnabled ? "ON" : "OFF")
                InspectorRow(label: "Gizmo", value: appState.interaction.gizmoEnabled ? "ON" : "OFF")
            }

            if context.workbench == .model {
                InspectorGroup(title: localized("Функция", "Feature")) {
                    InspectorRow(label: localized("Объект", "Object"), value: context.selection.hasSelection ? selectionKindTitle : localized("Не выбрано", "Nothing selected"))
                    InspectorRow(label: localized("Источник", "Source"), value: "MirEngine")
                }
            }
        }
    }

    private var constraintsContent: some View {
        InspectorGroup(title: localized("Ограничения эскиза", "Sketch Constraints")) {
            InspectorRow(label: localized("Выбрано элементов", "Selected elements"), value: "\(context.selection.count)")
            InspectorRow(label: localized("Режим", "Mode"), value: localizedSubMode)
            InspectorRow(label: localized("Состояние", "State"), value: localized("Готово к ограничению", "Ready for constraint"))
        }
    }

    private var dimensionsContent: some View {
        InspectorGroup(title: localized("Размеры", "Dimensions")) {
            InspectorRow(label: localized("Выбрано", "Selected"), value: "\(context.selection.count)")
            InspectorRow(label: localized("Единицы", "Units"), value: "mm")
            InspectorRow(label: localized("Тип", "Type"), value: localized("Параметрический", "Parametric"))
        }
    }

    private var materialContent: some View {
        InspectorGroup(title: localized("Материал", "Material")) {
            InspectorRow(label: localized("Объект", "Object"), value: context.selection.hasSelection ? selectionKindTitle : localized("Не выбрано", "Nothing selected"))
            InspectorRow(label: localized("Материал", "Material"), value: "—")
            InspectorRow(label: localized("Источник", "Source"), value: "MirEngine")
        }
    }

    private var simulationFieldsContent: some View {
        InspectorGroup(title: localized("Результирующие поля", "Result Fields")) {
            InspectorRow(label: localized("Физика", "Physics"), value: physicsTitle)
            InspectorRow(label: localized("Набор", "Result set"), value: appState.simulation.resultSetID ?? "—")
            InspectorRow(label: localized("Статус", "Status"), value: appState.simulation.solverStatus)
        }
    }

    private var contoursContent: some View {
        InspectorGroup(title: localized("Контуры", "Contours")) {
            InspectorRow(label: localized("Физика", "Physics"), value: physicsTitle)
            InspectorRow(label: localized("Шкала", "Scale"), value: localized("Авто", "Auto"))
            InspectorRow(label: localized("Диапазон", "Range"), value: "—")
        }
    }

    private var sensorsContent: some View {
        InspectorGroup(title: localized("Датчики", "Sensors")) {
            InspectorRow(label: localized("Активно", "Active"), value: "0")
            InspectorRow(label: localized("Сигналы", "Signals"), value: "—")
            InspectorRow(label: localized("Источник", "Source"), value: "MirEngine")
        }
    }

    private var timeContent: some View {
        InspectorGroup(title: localized("Состояние 4D", "4D State")) {
            InspectorRow(label: "T", value: String(format: "%.3f s", appState.currentTime))
            InspectorRow(label: localized("Прогресс", "Progress"), value: String(format: "%.1f%%", appState.timeState.normalizedProgress * 100))
            InspectorRow(label: localized("Сценарий", "Scenario"), value: appState.timeState.scenarioID)
            InspectorRow(label: localized("Ветка", "Branch"), value: appState.timeState.branchID)
            InspectorRow(label: localized("Воспроизведение", "Playback"), value: appState.isPlaying ? "PLAY" : "PAUSE")
        }
    }

    private var eventsContent: some View {
        InspectorGroup(title: localized("Временные события", "Time Events")) {
            InspectorRow(label: localized("Текущий момент", "Current time"), value: String(format: "%.3f s", appState.currentTime))
            InspectorRow(label: localized("Событий", "Events"), value: "0")
            InspectorRow(label: localized("Следующее", "Next"), value: "—")
        }
    }

    private var branchesContent: some View {
        InspectorGroup(title: localized("Сценарии и ветки", "Scenarios & Branches")) {
            InspectorRow(label: localized("Сценарий", "Scenario"), value: appState.timeState.scenarioID)
            InspectorRow(label: localized("Ветка", "Branch"), value: appState.timeState.branchID)
            InspectorRow(label: localized("Состояние", "State"), value: localized("Активна", "Active"))
        }
    }

    private var historyContent: some View {
        InspectorGroup(title: localized("История", "History")) {
            InspectorRow(label: localized("Документ", "Document"), value: appState.documentName)
            InspectorRow(label: localized("Изменён", "Dirty"), value: appState.documentDirty ? "YES" : "NO")
            InspectorRow(label: localized("Сценарий", "Scenario"), value: appState.timeState.scenarioID)
            InspectorRow(label: localized("Ветка", "Branch"), value: appState.timeState.branchID)
        }
    }

    private var iconForSelection: String {
        switch context.selection.primaryKind {
        case .none:
            return "cursorarrow"

        case .vertex:
            return "circle.fill"

        case .edge:
            return "line.diagonal"

        case .face:
            return "square"

        case .body:
            return "cube"

        case .feature:
            return "wand.and.stars"

        case .sketch:
            return "pencil.and.ruler"

        case .component:
            return "square.stack.3d.up"

        case .simulationResult:
            return "waveform.path.ecg"

        case .drawingView:
            return "doc.text"

        case .unknown:
            return "questionmark.circle"
        }
    }
    
    
    private var contextTitle: String {
        if context.selection.hasSelection {
            return selectionKindTitle
        }
        return localizedWorkbench
    }

    private var localizedWorkbench: String {
        appState.ui.language == .russian ? appState.workbench.titleRU : appState.workbench.titleEN
    }

    private var localizedSubMode: String {
        appState.ui.language == .russian ? appState.subMode.titleRU : appState.subMode.titleEN
    }

    private var physicsTitle: String {
        appState.ui.language == .russian
            ? appState.simulation.physics.titleRU
            : appState.simulation.physics.titleEN
    }

    private var selectionSubtitle: String {
        let count = context.selection.count
        if count == 0 {
            return appState.ui.language == .russian ? "Ничего не выбрано" : "Nothing selected"
        }
        if context.selection.isMultiSelection {
            return appState.ui.language == .russian ? "Множественный выбор · общие свойства" : "Multi-selection · common properties"
        }
        return appState.ui.language == .russian ? "Выбран инженерный объект" : "Engineering object selected"
    }

    private var selectionKindTitle: String {
        switch context.selection.primaryKind {
        case .none: return localized("Нет", "None")
        case .vertex: return localized("Вершина", "Vertex")
        case .edge: return localized("Ребро", "Edge")
        case .face: return localized("Грань", "Face")
        case .body: return localized("Тело", "Body")
        case .feature: return localized("Функция", "Feature")
        case .sketch: return localized("Эскиз", "Sketch")
        case .component: return localized("Компонент", "Component")
        case .simulationResult: return localized("Результат расчёта", "Simulation result")
        case .drawingView: return localized("Вид чертежа", "Drawing view")
        case .unknown: return localized("Инженерный объект", "Engineering object")
        }
    }

    private func localized(_ ru: String, _ en: String) -> String {
        appState.ui.language == .russian ? ru : en
    }

    private func ensureValidTab() {
        if !tabs.contains(activeTab) {
            activeTab = tabs.first ?? .parameters
        }
    }
}

struct InspectorGroup<Content: View>: View {
    let title: String
    @ViewBuilder let content: Content

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(title.uppercased())
                .font(.system(size: 9, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .tracking(0.4)

            content
        }
    }
}

struct InspectorRow: View {
    let label: String
    let value: String

    var body: some View {
        HStack {
            Text(label)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer()
            Text(value)
                .font(MirTheme.Typography.numeric)
                .foregroundStyle(MirTheme.Colors.textPrimary)
                .lineLimit(1)
        }
        .padding(.vertical, 5)
        .overlay(alignment: .bottom) {
            Rectangle()
                .fill(MirTheme.Colors.border.opacity(0.45))
                .frame(height: 1)
        }
    }
}
