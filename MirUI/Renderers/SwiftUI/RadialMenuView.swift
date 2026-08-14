import SwiftUI

extension Notification.Name {
    static let mir4DRadialMenuBegan = Notification.Name("MIR4D.RadialMenuBegan")
    static let mir4DRadialMenuMoved = Notification.Name("MIR4D.RadialMenuMoved")
    static let mir4DRadialMenuEnded = Notification.Name("MIR4D.RadialMenuEnded")
}

struct RadialMenuTool: Codable, Identifiable, Hashable {
    var id: UUID = UUID()
    var title: String
    var icon: String
    var command: String
}

struct RadialMenuPanel: Codable, Identifiable, Hashable {
    var id: UUID = UUID()
    var title: String
    var icon: String
    var enabled: Bool = true
    var tools: [RadialMenuTool]
}

struct RadialMenuSettings: Codable {
    var enabled = true
    var holdDuration: Double = 0.18
    var deadZone: Double = 28
    var panelRadius: Double = 82
    var submenuOffset: Double = 128
    var activationRadius: Double = 172
    var sectorGap: Double = 0.10
    var showLabels = true
    var hapticEnabled = true
    var keyboardTriggerEnabled = true
    var middleMouseTriggerEnabled = true
    var leftMouseHoldTriggerEnabled = true
    var panels: [RadialMenuPanel] = RadialMenuSettings.defaultPanels

    static let defaultPanels: [RadialMenuPanel] = [
        RadialMenuPanel(title: "Модель", icon: "cube", tools: [
            RadialMenuTool(title: "Новое тело", icon: "cube.transparent", command: "create.body"),
            RadialMenuTool(title: "Эскиз", icon: "pencil.and.ruler", command: "create.sketch"),
            RadialMenuTool(title: "Extrude", icon: "arrow.up", command: "feature.extrude"),
            RadialMenuTool(title: "Move", icon: "arrow.up.and.down.and.arrow.left.and.right", command: "transform.move")
        ]),
        RadialMenuPanel(title: "Сборка", icon: "square.stack.3d.up", tools: [
            RadialMenuTool(title: "Компонент", icon: "shippingbox", command: "assembly.component"),
            RadialMenuTool(title: "Связь", icon: "link", command: "assembly.constraint"),
            RadialMenuTool(title: "Вставить", icon: "arrow.down.doc", command: "assembly.insert")
        ]),
        RadialMenuPanel(title: "Симуляция", icon: "waveform.path.ecg", tools: [
            RadialMenuTool(title: "Запуск", icon: "play.fill", command: "simulation.run"),
            RadialMenuTool(title: "Пауза", icon: "pause.fill", command: "simulation.pause"),
            RadialMenuTool(title: "Результаты", icon: "chart.bar.xaxis", command: "simulation.results")
        ]),
        RadialMenuPanel(title: "4D", icon: "clock.arrow.circlepath", tools: [
            RadialMenuTool(title: "Сценарий", icon: "point.3.connected.trianglepath.dotted", command: "fourD.scenario"),
            RadialMenuTool(title: "Время", icon: "clock", command: "fourD.timeline"),
            RadialMenuTool(title: "Ветка", icon: "arrow.triangle.branch", command: "fourD.branch")
        ]),
        RadialMenuPanel(title: "Чертёж", icon: "doc.text.magnifyingglass", tools: [
            RadialMenuTool(title: "Вид", icon: "viewfinder", command: "drawing.view"),
            RadialMenuTool(title: "Размер", icon: "ruler", command: "drawing.dimension"),
            RadialMenuTool(title: "Выпуск", icon: "checkmark.seal", command: "drawing.release")
        ]),
        RadialMenuPanel(title: "Производство", icon: "hammer", tools: [
            RadialMenuTool(title: "BOM", icon: "list.bullet.rectangle", command: "manufacturing.bom"),
            RadialMenuTool(title: "Маршрут", icon: "point.3.connected.trianglepath.dotted", command: "manufacturing.route"),
            RadialMenuTool(title: "Отправить", icon: "paperplane.fill", command: "manufacturing.submit")
        ]),
        RadialMenuPanel(title: "Файл", icon: "doc", tools: [
            RadialMenuTool(title: "Импорт", icon: "square.and.arrow.down", command: "file.import"),
            RadialMenuTool(title: "Экспорт", icon: "square.and.arrow.up", command: "file.export"),
            RadialMenuTool(title: "Сохранить", icon: "square.and.arrow.down.on.square", command: "file.save")
        ]),
        RadialMenuPanel(title: "Вид", icon: "eye", tools: [
            RadialMenuTool(title: "Fit", icon: "arrow.up.left.and.arrow.down.right", command: "view.fit"),
            RadialMenuTool(title: "Изометрия", icon: "cube", command: "view.isometric"),
            RadialMenuTool(title: "Ортографический", icon: "square.split.2x2", command: "view.orthographic")
        ])
    ]
}

@MainActor
final class RadialMenuSettingsStore: ObservableObject {
    static let shared = RadialMenuSettingsStore()

    @Published var settings: RadialMenuSettings {
        didSet { save() }
    }

    private let key = "MIR4D.RadialMenu.Settings"

    private init() {
        if let data = UserDefaults.standard.data(forKey: key),
           let value = try? JSONDecoder().decode(RadialMenuSettings.self, from: data) {
            settings = value
        } else {
            settings = RadialMenuSettings()
        }
    }

    func reset() {
        settings = RadialMenuSettings()
    }

    private func save() {
        guard let data = try? JSONEncoder().encode(settings) else { return }
        UserDefaults.standard.set(data, forKey: key)
    }
}

enum RadialMenuGeometry {
    static func normalizedAngle(_ angle: Double) -> Double {
        var value = angle.truncatingRemainder(dividingBy: Double.pi * 2)
        if value < 0 { value += Double.pi * 2 }
        return value
    }

    static func enabledPanels(_ settings: RadialMenuSettings) -> [RadialMenuPanel] {
        settings.panels.filter(\.enabled)
    }

    static func panelIndex(for dx: Double, dy: Double, settings: RadialMenuSettings) -> Int? {
        let panels = enabledPanels(settings)
        guard !panels.isEmpty, hypot(dx, dy) >= settings.deadZone else { return nil }
        let angle = normalizedAngle(atan2(-dy, dx) + Double.pi / 2)
        let sector = Double.pi * 2 / Double(panels.count)
        return min(Int((angle / sector).rounded(.down)), panels.count - 1)
    }

    static func toolIndex(for dx: Double, dy: Double, panel: RadialMenuPanel, settings: RadialMenuSettings) -> Int? {
        guard hypot(dx, dy) >= settings.activationRadius, !panel.tools.isEmpty else { return nil }
        let angle = normalizedAngle(atan2(-dy, dx) + Double.pi / 2)
        let sector = Double.pi * 2 / Double(panel.tools.count)
        return min(Int((angle / sector).rounded(.down)), panel.tools.count - 1)
    }
}

struct RadialMenuView: View {
    @ObservedObject var store: RadialMenuSettingsStore
    let center: CGPoint
    let vector: CGVector
    let onToolActivated: (RadialMenuTool) -> Void
    let onSettings: () -> Void

    private var panels: [RadialMenuPanel] { RadialMenuGeometry.enabledPanels(store.settings) }
    private var selectedPanelIndex: Int? {
        RadialMenuGeometry.panelIndex(for: vector.dx, dy: vector.dy, settings: store.settings)
    }
    private var selectedPanel: RadialMenuPanel? {
        guard let selectedPanelIndex, panels.indices.contains(selectedPanelIndex) else { return nil }
        return panels[selectedPanelIndex]
    }
    private var selectedToolIndex: Int? {
        guard let selectedPanel else { return nil }
        return RadialMenuGeometry.toolIndex(for: vector.dx, dy: vector.dy, panel: selectedPanel, settings: store.settings)
    }

    var body: some View {
        ZStack {
            centerCore
            panelRing
            if let selectedPanel {
                toolRing(panel: selectedPanel)
            }
        }
        .frame(width: 520, height: 520)
        .position(center)
        .allowsHitTesting(false)
        .transition(.opacity.combined(with: .scale(scale: 0.88)))
    }

    private var centerCore: some View {
        VStack(spacing: 4) {
            Image(systemName: "cursorarrow.motionlines")
                .font(.system(size: 18, weight: .semibold))
            Text(selectedPanel?.title ?? "МИР")
                .font(.system(size: 10, weight: .bold))
            if selectedPanelIndex != nil && selectedToolIndex != nil {
                Text(selectedPanel!.tools[selectedToolIndex!].title)
                    .font(.system(size: 9))
                    .foregroundStyle(MirTheme.Colors.selection)
            }
        }
        .foregroundStyle(.white)
        .frame(width: 68, height: 68)
        .background(.ultraThinMaterial, in: Circle())
        .overlay(Circle().stroke(MirTheme.Colors.accentBright.opacity(0.8), lineWidth: 1))
        .overlay(alignment: .bottomTrailing) {
            Button(action: onSettings) {
                Image(systemName: "gearshape.fill")
                    .font(.system(size: 10))
                    .foregroundStyle(.white.opacity(0.8))
                    .padding(7)
                    .background(.black.opacity(0.45), in: Circle())
            }
            .buttonStyle(.plain)
            .allowsHitTesting(true)
        }
    }

    private var panelRing: some View {
        ZStack {
            Circle()
                .stroke(Color.white.opacity(0.10), lineWidth: 1)
                .frame(width: store.settings.panelRadius * 2.1, height: store.settings.panelRadius * 2.1)

            ForEach(Array(panels.enumerated()), id: \.element.id) { index, panel in
                let angle = sectorAngle(index: index, count: panels.count)
                let selected = selectedPanelIndex == index
                radialButton(
                    title: panel.title,
                    icon: panel.icon,
                    selected: selected,
                    position: point(radius: store.settings.panelRadius, angle: angle)
                )
            }
        }
    }

    private func toolRing(panel: RadialMenuPanel) -> some View {
        let direction = CGVector(dx: vector.dx, dy: vector.dy)
        let length = max(hypot(direction.dx, direction.dy), 1)
        let ux = direction.dx / length
        let uy = direction.dy / length
        let origin = CGPoint(
            x: 260 + ux * store.settings.submenuOffset,
            y: 260 + uy * store.settings.submenuOffset
        )

        return ZStack {
            Circle()
                .stroke(MirTheme.Colors.selection.opacity(0.32), lineWidth: 1)
                .frame(width: 116, height: 116)
                .position(origin)

            ForEach(Array(panel.tools.enumerated()), id: \.element.id) { index, tool in
                let angle = sectorAngle(index: index, count: panel.tools.count)
                let selected = selectedToolIndex == index
                radialButton(
                    title: store.settings.showLabels ? tool.title : "",
                    icon: tool.icon,
                    selected: selected,
                    position: CGPoint(
                        x: origin.x + CGFloat(cos(angle)) * 52,
                        y: origin.y + CGFloat(sin(angle)) * 52
                    )
                )
            }
        }
    }

    private func radialButton(title: String, icon: String, selected: Bool, position: CGPoint) -> some View {
        VStack(spacing: 2) {
            Image(systemName: icon)
                .font(.system(size: selected ? 15 : 12, weight: .semibold))
            if !title.isEmpty {
                Text(title)
                    .font(.system(size: 8, weight: .semibold))
                    .lineLimit(1)
            }
        }
        .foregroundStyle(selected ? .white : Color.white.opacity(0.74))
        .frame(width: selected ? 74 : 64, height: selected ? 48 : 42)
        .background(
            selected ? MirTheme.Colors.selection.opacity(0.76) : Color.black.opacity(0.42),
            in: RoundedRectangle(cornerRadius: 12)
        )
        .overlay(
            RoundedRectangle(cornerRadius: 12)
                .stroke(selected ? MirTheme.Colors.selection : Color.white.opacity(0.12), lineWidth: selected ? 1.5 : 0.8)
        )
        .position(position)
    }

    private func sectorAngle(index: Int, count: Int) -> Double {
        let step = Double.pi * 2 / Double(max(count, 1))
        return -Double.pi / 2 + step * Double(index)
    }

    private func point(radius: Double, angle: Double) -> CGPoint {
        CGPoint(
            x: 260 + CGFloat(cos(angle) * radius),
            y: 260 + CGFloat(sin(angle) * radius)
        )
    }
}

struct RadialMenuSettingsView: View {
    @ObservedObject var store: RadialMenuSettingsStore

    var body: some View {
        NavigationStack {
            Form {
                Section("Управление") {
                    Toggle("Радиальное меню включено", isOn: binding(\.enabled))
                    Toggle("Средняя кнопка мыши", isOn: binding(\.middleMouseTriggerEnabled))
                    Toggle("Удержание левой кнопки", isOn: binding(\.leftMouseHoldTriggerEnabled))
                    Toggle("Клавиша `", isOn: binding(\.keyboardTriggerEnabled))
                    Toggle("Тактическая обратная связь", isOn: binding(\.hapticEnabled))
                }

                Section("Геометрия и жест") {
                    slider("Зона покоя", keyPath: \.deadZone, range: 8...80, step: 1, suffix: " pt")
                    slider("Радиус панелей", keyPath: \.panelRadius, range: 50...150, step: 1, suffix: " pt")
                    slider("Смещение подменю", keyPath: \.submenuOffset, range: 80...220, step: 1, suffix: " pt")
                    slider("Радиус активации", keyPath: \.activationRadius, range: 110...280, step: 1, suffix: " pt")
                    slider("Задержка удержания", keyPath: \.holdDuration, range: 0.05...0.5, step: 0.01, suffix: " s")
                    Toggle("Подписи инструментов", isOn: binding(\.showLabels))
                }

                Section("Панели и инструменты") {
                    ForEach(store.settings.panels.indices, id: \.self) { index in
                        PanelEditor(store: store, index: index)
                    }
                }

                Section {
                    Button("Восстановить заводскую схему") {
                        store.reset()
                    }
                }
            }
            .navigationTitle("Настройки радиального меню")
            .formStyle(.grouped)
        }
        .frame(minWidth: 560, minHeight: 620)
    }

    private func binding<T>(_ keyPath: WritableKeyPath<RadialMenuSettings, T>) -> Binding<T> {
        Binding(
            get: { store.settings[keyPath: keyPath] },
            set: { store.settings[keyPath: keyPath] = $0 }
        )
    }

    private func slider(_ title: String, keyPath: WritableKeyPath<RadialMenuSettings, Double>, range: ClosedRange<Double>, step: Double, suffix: String) -> some View {
        VStack(alignment: .leading) {
            HStack {
                Text(title)
                Spacer()
                Text(String(format: "%.2f%@", store.settings[keyPath: keyPath], suffix))
                    .foregroundStyle(MirTheme.Colors.textSecondary)
            }
            Slider(value: binding(keyPath), in: range, step: step)
        }
    }
}

private struct PanelEditor: View {
    @ObservedObject var store: RadialMenuSettingsStore
    let index: Int
    @State private var toolsText = ""

    var body: some View {
        let panel = store.settings.panels[index]
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Image(systemName: panel.icon)
                TextField("Название панели", text: panelTitleBinding)
                Toggle("Вкл", isOn: panelEnabledBinding)
                    .labelsHidden()
            }
            TextField("Инструменты через запятую", text: toolsBinding)
                .textFieldStyle(.roundedBorder)
        }
        .onAppear { toolsText = panel.tools.map(\.title).joined(separator: ", ") }
    }

    private var panelTitleBinding: Binding<String> {
        Binding(
            get: { store.settings.panels[index].title },
            set: { store.settings.panels[index].title = $0 }
        )
    }

    private var panelEnabledBinding: Binding<Bool> {
        Binding(
            get: { store.settings.panels[index].enabled },
            set: { store.settings.panels[index].enabled = $0 }
        )
    }

    private var toolsBinding: Binding<String> {
        Binding(
            get: { toolsText },
            set: { newValue in
                toolsText = newValue
                let titles = newValue.split(separator: ",").map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }.filter { !$0.isEmpty }
                store.settings.panels[index].tools = titles.map { title in
                    RadialMenuTool(title: title, icon: "circle", command: "custom.\(title.lowercased().replacingOccurrences(of: " ", with: "."))")
                }
            }
        )
    }
}
