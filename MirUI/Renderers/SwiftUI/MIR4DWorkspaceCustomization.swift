import SwiftUI

/// Presentation-only preferences for the MIR 4D workspace.
/// Engineering data remains owned by MirEngine/CADAppState.
@MainActor
final class MIR4DWorkspaceCustomizationStore: ObservableObject {
    static let shared = MIR4DWorkspaceCustomizationStore()

    @Published var compactPanels: Bool { didSet { save() } }
    @Published var showPanelHeaders: Bool { didSet { save() } }
    @Published var panelOpacity: Double { didSet { save() } }
    @Published var panelCornerRadius: Double { didSet { save() } }
    @Published var leftWidth: Double { didSet { save() } }
    @Published var rightWidth: Double { didSet { save() } }
    @Published var bottomHeight: Double { didSet { save() } }
    @Published var floatingWidth: Double { didSet { save() } }
    @Published var floatingHeight: Double { didSet { save() } }

    private init() {
        let d = UserDefaults.standard
        compactPanels = d.object(forKey: "mir4d.workspace.compact") as? Bool ?? false
        showPanelHeaders = d.object(forKey: "mir4d.workspace.headers") as? Bool ?? true
        panelOpacity = d.object(forKey: "mir4d.workspace.opacity") as? Double ?? 1
        panelCornerRadius = d.object(forKey: "mir4d.workspace.radius") as? Double ?? 6
        leftWidth = d.object(forKey: "mir4d.workspace.left") as? Double ?? 236
        rightWidth = d.object(forKey: "mir4d.workspace.right") as? Double ?? 280
        bottomHeight = d.object(forKey: "mir4d.workspace.bottom") as? Double ?? 240
        floatingWidth = d.object(forKey: "mir4d.workspace.floatingWidth") as? Double ?? 420
        floatingHeight = d.object(forKey: "mir4d.workspace.floatingHeight") as? Double ?? 520
    }

    func reset() {
        compactPanels = false
        showPanelHeaders = true
        panelOpacity = 1
        panelCornerRadius = 6
        leftWidth = 236
        rightWidth = 280
        bottomHeight = 240
        floatingWidth = 420
        floatingHeight = 520
        FloatingPanelManager.shared.applySizeToAll(width: floatingWidth, height: floatingHeight)
    }

    private func save() {
        let d = UserDefaults.standard
        d.set(compactPanels, forKey: "mir4d.workspace.compact")
        d.set(showPanelHeaders, forKey: "mir4d.workspace.headers")
        d.set(panelOpacity, forKey: "mir4d.workspace.opacity")
        d.set(panelCornerRadius, forKey: "mir4d.workspace.radius")
        d.set(leftWidth, forKey: "mir4d.workspace.left")
        d.set(rightWidth, forKey: "mir4d.workspace.right")
        d.set(bottomHeight, forKey: "mir4d.workspace.bottom")
        d.set(floatingWidth, forKey: "mir4d.workspace.floatingWidth")
        d.set(floatingHeight, forKey: "mir4d.workspace.floatingHeight")
    }
}

/// Full live editor for MirUI. All changes are presentation-only.
struct MIR4DWorkspaceCustomizationView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var workspace = MIR4DWorkspaceCustomizationStore.shared
    @ObservedObject private var appearance = MirUIAppearanceStore.shared
    @Environment(\.dismiss) private var dismiss

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        NavigationStack {
            ScrollView {
                VStack(alignment: .leading, spacing: 18) {
                    liveBanner
                    panelsSection
                    dimensionsSection
                    appearanceSection
                    interactionSection
                    resetSection
                }
                .padding(24)
            }
            .background(appearance.theme.windowBackground)
            .navigationTitle(russian ? "Редактор интерфейса МИР 4D" : "MIR 4D Interface Editor")
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button(russian ? "Готово" : "Done") { dismiss() }
                }
            }
        }
        .frame(minWidth: 900, idealWidth: 980, minHeight: 700, idealHeight: 760)
        .preferredColorScheme(appearance.theme.colorScheme)
        .tint(appearance.theme.accent)
        .onChange(of: workspace.floatingWidth) { _, value in
            FloatingPanelManager.shared.applySizeToAll(width: value, height: workspace.floatingHeight)
        }
        .onChange(of: workspace.floatingHeight) { _, value in
            FloatingPanelManager.shared.applySizeToAll(width: workspace.floatingWidth, height: value)
        }
    }

    private var liveBanner: some View {
        HStack(spacing: 12) {
            Image(systemName: "hand.draw.fill")
                .font(.system(size: 18, weight: .semibold))
                .foregroundStyle(appearance.theme.accent)
            VStack(alignment: .leading, spacing: 3) {
                Text(russian ? "Живой редактор рабочего пространства" : "Live workspace editor")
                    .font(.headline)
                Text(russian ? "Изменения применяются сразу. Панели можно скрывать, перемещать и переводить в отдельные изменяемые окна." : "Changes apply immediately. Panels can be hidden, moved and detached into resizable windows.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            Spacer()
        }
        .padding(16)
        .background(appearance.theme.accent.opacity(0.10), in: RoundedRectangle(cornerRadius: 12))
    }

    private var panelsSection: some View {
        VStack(alignment: .leading, spacing: 10) {
            sectionTitle(russian ? "ПАНЕЛИ И РАСПОЛОЖЕНИЕ" : "PANELS & PLACEMENT")
            ForEach(CADPanel.allCases) { panel in
                HStack(spacing: 12) {
                    Image(systemName: icon(for: panel)).frame(width: 28, height: 28)
                    VStack(alignment: .leading, spacing: 2) {
                        Text(russian ? panel.titleRU : panel.titleEN).font(.subheadline.weight(.semibold))
                        Text(russian ? appState.panelPlacement(for: panel).titleRU : appState.panelPlacement(for: panel).titleEN)
                            .font(.caption).foregroundStyle(.secondary)
                    }
                    Spacer()
                    Picker("", selection: Binding(
                        get: { appState.panelPlacement(for: panel) },
                        set: { appState.setPanelPlacement($0, for: panel) }
                    )) {
                        ForEach(PanelPlacement.allCases) { placement in
                            Text(russian ? placement.titleRU : placement.titleEN).tag(placement)
                        }
                    }
                    .labelsHidden()
                    .frame(width: 155)
                    Toggle("", isOn: Binding(
                        get: { appState.visiblePanels.contains(panel) },
                        set: { _ in appState.togglePanel(panel) }
                    ))
                    .labelsHidden().toggleStyle(.switch).controlSize(.small)
                }
                .padding(11)
                .background(Color.primary.opacity(0.045), in: RoundedRectangle(cornerRadius: 9))
            }
        }
    }

    private var dimensionsSection: some View {
        VStack(alignment: .leading, spacing: 12) {
            sectionTitle(russian ? "РАЗМЕРЫ" : "DIMENSIONS")
            dimensionRow(russian ? "Левая зона" : "Left zone", value: $workspace.leftWidth, range: 180...420)
            dimensionRow(russian ? "Правая зона" : "Right zone", value: $workspace.rightWidth, range: 220...480)
            dimensionRow(russian ? "Нижняя зона" : "Bottom zone", value: $workspace.bottomHeight, range: 140...420)
            Divider()
            dimensionRow(russian ? "Плавающее окно — ширина" : "Floating window — width", value: $workspace.floatingWidth, range: 300...900)
            dimensionRow(russian ? "Плавающее окно — высота" : "Floating window — height", value: $workspace.floatingHeight, range: 280...900)
        }
    }

    private func dimensionRow(_ title: String, value: Binding<Double>, range: ClosedRange<Double>) -> some View {
        HStack {
            Text(title).font(.subheadline)
            Spacer()
            Slider(value: value, in: range, step: 1).frame(width: 260)
            Text("\(Int(value.wrappedValue)) px")
                .font(.system(.caption, design: .monospaced))
                .frame(width: 70, alignment: .trailing)
        }
    }

    private var appearanceSection: some View {
        VStack(alignment: .leading, spacing: 12) {
            sectionTitle(russian ? "ВНЕШНИЙ ВИД" : "APPEARANCE")
            HStack {
                Text(russian ? "Тема" : "Theme")
                Spacer()
                Picker("", selection: Binding(get: { appearance.theme }, set: { appearance.setTheme($0) })) {
                    ForEach(MirUIAppearanceStore.Theme.allCases) { theme in
                        Text(russian ? theme.titleRU : theme.titleEN).tag(theme)
                    }
                }
                .labelsHidden().frame(width: 190)
            }
            dimensionRow(russian ? "Прозрачность панелей" : "Panel opacity", value: $workspace.panelOpacity, range: 0.55...1)
            dimensionRow(russian ? "Скругление" : "Corner radius", value: $workspace.panelCornerRadius, range: 0...18)
            Toggle(russian ? "Показывать заголовки панелей" : "Show panel headers", isOn: $workspace.showPanelHeaders)
            Toggle(russian ? "Компактный режим панелей" : "Compact panel mode", isOn: $workspace.compactPanels)
        }
    }

    private var interactionSection: some View {
        VStack(alignment: .leading, spacing: 10) {
            sectionTitle(russian ? "ИНТЕРАКТИВНОСТЬ" : "INTERACTION")
            HStack(spacing: 10) {
                moveButton("arrow.left", russian ? "Все слева" : "All left", .left)
                moveButton("arrow.right", russian ? "Все справа" : "All right", .right)
                moveButton("arrow.down", russian ? "Все вниз" : "All bottom", .bottom)
                moveButton("macwindow.on.rectangle", russian ? "Все плавающие" : "All floating", .floating)
            }
            Text(russian ? "Плавающие панели — полноценные изменяемые окна macOS: их можно свободно перемещать и менять размер мышью." : "Floating panels are resizable macOS windows: move and resize them freely with the mouse.")
                .font(.caption).foregroundStyle(.secondary)
        }
    }

    private func moveButton(_ icon: String, _ title: String, _ placement: PanelPlacement) -> some View {
        Button {
            for panel in appState.visiblePanels { appState.setPanelPlacement(placement, for: panel) }
        } label: {
            Label(title, systemImage: icon).frame(maxWidth: .infinity)
        }
        .buttonStyle(.bordered)
    }

    private var resetSection: some View {
        HStack {
            Button(role: .destructive) {
                workspace.reset()
                appState.panelState = .forWorkbench(appState.workbench)
                appearance.reset()
            } label: {
                Label(russian ? "Сбросить интерфейс" : "Reset interface", systemImage: "arrow.counterclockwise")
            }
            Spacer()
            Text(russian ? "Настройки сохраняются локально" : "Preferences are stored locally")
                .font(.caption).foregroundStyle(.secondary)
        }
    }

    private func sectionTitle(_ title: String) -> some View {
        Text(title).font(.caption.weight(.semibold)).tracking(0.6).foregroundStyle(.secondary)
    }

    private func icon(for panel: CADPanel) -> String {
        switch panel {
        case .project: return "folder"
        case .properties: return "slider.horizontal.3"
        case .timeline: return "clock"
        case .simulation: return "waveform.path.ecg"
        case .history: return "clock.arrow.circlepath"
        case .aiInspector: return "sparkles"
        }
    }
}
