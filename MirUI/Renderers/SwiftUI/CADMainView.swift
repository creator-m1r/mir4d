import SwiftUI
import AppKit
import UniformTypeIdentifiers

private let mir4DImportTypes: [UTType] = [
    UTType(filenameExtension: "stl", conformingTo: .data),
    UTType(filenameExtension: "obj", conformingTo: .data),
    UTType(filenameExtension: "ply", conformingTo: .data),
    UTType(filenameExtension: "gltf", conformingTo: .data),
    UTType(filenameExtension: "glb", conformingTo: .data),
    UTType(filenameExtension: "fbx", conformingTo: .data)
].compactMap { $0 }

private let mir4DSTLType: UTType = {
    UTType(filenameExtension: "stl", conformingTo: .data)
        ?? .data
}()

struct CADMainView: View {
    @ObservedObject var appState: CADAppState
    @StateObject private var commandRegistry = CADCommandRegistry()
    @StateObject private var appearance = MirUIAppearanceStore.shared
    @StateObject private var productionWorld = ProductionWorldStore()
    @StateObject private var radialMenuSettings = RadialMenuSettingsStore.shared
    @StateObject private var radialContextPolicies = RadialMenuContextPolicyStore.shared
    @StateObject private var radialContextStore = RadialMenuContextStore.shared
    @ObservedObject private var modelRuntime = MIR4DModelRuntime.shared

    @State private var commandPalettePresented = false
    @State private var createBodyPresented = false
    @State private var radialSettingsPresented = false
    @State private var radialMenuVisible = false
    @State private var radialCenterNormalized = CGPoint(x: 0.5, y: 0.5)
    @State private var radialVector = CGVector(dx: 0, dy: 0)
    @State private var radialBasePanels: [RadialMenuPanel]?
    @State private var cameraTheta = 0.8
    @State private var cameraPhi = 1.2
    @State private var cameraDistance = 12.0
    @State private var draggingPanel: CADPanel?
    @State private var dropTarget: PanelPlacement?

    var body: some View {
        ZStack {
            appearance.theme.windowBackground.ignoresSafeArea()
            VStack(spacing: 0) {
                TopBarView(appState: appState, registry: commandRegistry, commandPalettePresented: $commandPalettePresented)
                mainLayout
                    .animation(MirTheme.Animation.normal, value: appState.visiblePanels)
                    .animation(MirTheme.Animation.normal, value: appState.panelState)
            }
            notificationsOverlay
            dropZones
        }
        .environment(\.locale, appearance.language.locale)
        .preferredColorScheme(appearance.theme.colorScheme)
        .tint(appearance.theme.accent)
        .sheet(isPresented: $commandPalettePresented) { CommandPaletteView(appState: appState, registry: commandRegistry) }
        .sheet(isPresented: $createBodyPresented) { CreateBodyView(appState: appState) }
        .sheet(isPresented: $radialSettingsPresented) { RadialMenuSettingsHubView(store: radialMenuSettings) }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuBegan)) { notification in
            guard let info = notification.userInfo, let x = info["x"] as? Double, let y = info["y"] as? Double else { return }
            radialCenterNormalized = CGPoint(x: x, y: y)
            radialVector = CGVector(dx: 0, dy: 0)
            applyContextualRadialLayout()
            radialMenuVisible = true
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuMoved)) { notification in
            guard let info = notification.userInfo, let dx = info["dx"] as? Double, let dy = info["dy"] as? Double else { return }
            radialVector = CGVector(dx: dx, dy: dy)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuEnded)) { notification in
            guard let info = notification.userInfo else { radialMenuVisible = false; restoreBaseRadialLayout(); return }
            let commit = (info["commit"] as? Bool) ?? false
            let dx = (info["dx"] as? Double) ?? radialVector.dx
            let dy = (info["dy"] as? Double) ?? radialVector.dy
            radialVector = CGVector(dx: dx, dy: dy)
            radialMenuVisible = false
            if commit { activateRadialSelection(dx: dx, dy: dy) }
            restoreBaseRadialLayout()
        }
        .onAppear {
            commandRegistry.registerDefaults(appState: appState)
            commandRegistry.registerExtendedScenarioCommands(appState: appState)
            FloatingPanelManager.shared.sync(appState: appState)
            appState.showNotification(appState.ui.language == .russian ? "МИР 4D готов · интерфейсный слой активирован" : "MIR 4D ready · interface layer activated", type: .success)
        }
        .onChange(of: appState.panelState) {
            FloatingPanelManager.shared.sync(appState: appState)
        }
    }

    private var leftPanels: [CADPanel] {
        appState.visiblePanels.filter { appState.panelPlacement(for: $0) == .left }.sorted { $0.rawValue < $1.rawValue }
    }

    private var rightPanels: [CADPanel] {
        appState.visiblePanels.filter { appState.panelPlacement(for: $0) == .right }.sorted { $0.rawValue < $1.rawValue }
    }

    private var bottomPanels: [CADPanel] {
        appState.visiblePanels.filter { appState.panelPlacement(for: $0) == .bottom }.sorted { $0.rawValue < $1.rawValue }
    }

    /// Dockable layout: left / center / right columns are resizable,
    /// bottom panels share the center column vertically.
    private var mainLayout: some View {
        HSplitView {
            if !leftPanels.isEmpty {
                sideColumn(leftPanels)
                    .frame(minWidth: 260, idealWidth: 300, maxWidth: 380)
            }
            centerColumn
            if !rightPanels.isEmpty {
                sideColumn(rightPanels)
                    .frame(minWidth: 300, idealWidth: 340, maxWidth: 440)
            }
        }
    }

    private func sideColumn(_ panels: [CADPanel]) -> some View {
        VStack(spacing: 0) {
            ForEach(panels) { panel in
                panelContainer(panel) { panelView(panel) }
            }
        }
    }

    private var centerColumn: some View {
        VSplitView {
            viewport.frame(minWidth: 320, idealHeight: 520, maxHeight: .infinity)
            if !bottomPanels.isEmpty {
                sideColumn(bottomPanels)
                    .frame(minWidth: 640, idealHeight: 230, maxHeight: 320)
            }
        }
    }

    @ViewBuilder private func panelView(_ panel: CADPanel) -> some View {
        CADPanelView(panel: panel, appState: appState)
    }

    private func panelContainer<Content: View>(_ panel: CADPanel, @ViewBuilder content: () -> Content) -> some View {
        content()
            .overlay(alignment: .topTrailing) { panelDragHandle(panel) }
    }

    private func panelDragHandle(_ panel: CADPanel) -> some View {
        Image(systemName: "line.3.horizontal")
            .font(.system(size: 10, weight: .semibold))
            .foregroundStyle(MirTheme.Colors.textTertiary)
            .padding(5)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.85), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay {
                RoundedRectangle(cornerRadius: MirTheme.Radius.small)
                    .stroke(MirTheme.Colors.border, lineWidth: 1)
            }
            .padding(6)
            .help(appState.ui.language == .russian ? "Перетащите, чтобы переместить панель" : "Drag to move panel")
            .onDrag {
                draggingPanel = panel
                dropTarget = nil
                return NSItemProvider(object: panel.rawValue as NSString)
            }
    }

    private var dropZones: some View {
        GeometryReader { geometry in
            if draggingPanel != nil {
                let zones: [(PanelPlacement, CGRect)] = [
                    (.left, CGRect(x: 0, y: 0, width: 170, height: geometry.size.height)),
                    (.right, CGRect(x: geometry.size.width - 170, y: 0, width: 170, height: geometry.size.height)),
                    (.bottom, CGRect(x: 0, y: geometry.size.height - 130, width: geometry.size.width, height: 130)),
                    (.floating, CGRect(x: geometry.size.width / 2 - 110, y: geometry.size.height / 2 - 70, width: 220, height: 140))
                ]
                ZStack {
                    ForEach(zones, id: \.0) { zone in
                        PanelDropZone(
                            placement: zone.0,
                            target: dropTarget,
                            onTargetChange: { isTargeted in dropTarget = isTargeted ? zone.0 : nil },
                            onDropPanel: { placement in
                                if let panel = draggingPanel { appState.setPanelPlacement(placement, for: panel) }
                                draggingPanel = nil
                                dropTarget = nil
                            }
                        )
                        .frame(width: zone.1.width, height: zone.1.height)
                        .position(x: zone.1.midX, y: zone.1.midY)
                    }
                }
            }
        }
    }

    private var viewport: some View {
        ZStack {
            MirGLView(
                onSelectionChanged: { objectId in
                    if objectId == 0 { appState.clearSelection() } else { appState.setSelection(ids: [String(objectId)], kind: .body) }
                },
                onIOError: { message in
                    appState.showNotification(userFacingErrorMessage(message), type: .error)
                    print("MIR4D IO: \(message)")
                },
                onCameraOrientationChanged: { theta, phi, distance in cameraTheta = theta; cameraPhi = phi; cameraDistance = distance }
            )
            .background(MirTheme.Colors.viewport)

            viewportOverlay
            navigationOverlay
            radialMenuOverlay

            if showEmptyState { emptyState }
        }
    }

    private var viewportOverlay: some View {
        VStack(spacing: 0) {
            HStack(alignment: .top, spacing: 0) {
                viewportToolbar
                Spacer()
                if appState.workbench == .fourD || appState.workbench == .simulation { FourDSceneOverlayView(appState: appState) }
            }
            Spacer()
            productionOverlays
            statusBar
        }
    }

    private var viewportToolbar: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack(spacing: 8) {
                SelectionFilterBar(appState: appState)
                Button { createBodyPresented = true } label: { Label(appState.ui.language == .russian ? "Новое тело" : "New Body", systemImage: "cube.transparent") }.buttonStyle(.borderedProminent).controlSize(.small)
                    .help(appState.ui.language == .russian ? "Создать тело" : "Create Body")
                Button { presentMeshImportPanel() } label: { Label(appState.ui.language == .russian ? "Импорт" : "Import", systemImage: "square.and.arrow.down") }.buttonStyle(.bordered).controlSize(.small)
                    .help(appState.ui.language == .russian ? "Импорт модели (STL, OBJ, PLY, GLTF, GLB, FBX)" : "Import model (STL, OBJ, PLY, GLTF, GLB, FBX)")
                Button { presentStlExportPanel(selectionOnly: false) } label: { Label(appState.ui.language == .russian ? "Экспорт" : "Export", systemImage: "square.and.arrow.up") }.buttonStyle(.bordered).controlSize(.small)
                    .help(appState.ui.language == .russian ? "Экспорт STL" : "Export STL")
                Button { NotificationCenter.default.post(name: .mir4DFitViewport, object: nil) } label: { Label(appState.ui.language == .russian ? "Подогнать" : "Fit All", systemImage: "arrow.up.left.and.down.right.magnifyingglass") }.buttonStyle(.bordered).controlSize(.small)
                    .help(appState.ui.language == .russian ? "Показать всё" : "Fit all objects")
                if appState.selection.hasSelection {
                    Button { presentStlExportPanel(selectionOnly: true) } label: { Label(appState.ui.language == .russian ? "Выбранное" : "Selected", systemImage: "arrow.up.doc") }.buttonStyle(.bordered).controlSize(.small)
                        .help(appState.ui.language == .russian ? "Экспортировать выбранное в STL" : "Export selection to STL")
                }
                viewOptionsMenu
                Button { radialSettingsPresented = true } label: { Image(systemName: "circle.grid.3x3.fill") }.buttonStyle(.bordered).controlSize(.small)
                    .help(appState.ui.language == .russian ? "Радиальное меню" : "Radial menu settings")
            }.padding(.top, 10).padding(.leading, 12)
            ContextualToolbarView(appState: appState, registry: commandRegistry).padding(.leading, 12)
            WorkbenchContentRouter(appState: appState, registry: commandRegistry)
        }
    }

    private var viewOptionsMenu: some View {
        Menu {
            Button {
                appState.toggleGrid()
            } label: {
                Label(appState.ui.language == .russian ? "Сетка" : "Grid", systemImage: "grid")
            }
            Button {
                appState.toggleAxes()
            } label: {
                Label(appState.ui.language == .russian ? "Оси" : "Axes", systemImage: "cube")
            }
            Divider()
            Button {
                appState.togglePanel(.project)
            } label: {
                Label(appState.ui.language == .russian ? "Модель" : "Model Tree", systemImage: "sidebar.left")
            }
            Button {
                appState.togglePanel(.properties)
            } label: {
                Label(appState.ui.language == .russian ? "Свойства" : "Inspector", systemImage: "sidebar.right")
            }
        } label: {
            Label(appState.ui.language == .russian ? "Вид" : "View", systemImage: "eye")
        }
        .menuStyle(.borderlessButton)
        .controlSize(.small)
        .help(appState.ui.language == .russian ? "Параметры вида" : "View options")
    }

    private var productionOverlays: some View {
        VStack(spacing: 0) {
            if productionWorld.activeStage == .idea { IdeaStudioView(appState: appState, store: productionWorld) }
            if productionWorld.activeStage == .test || productionWorld.activeStage == .scenario { DigitalWorldHUD(appState: appState, store: productionWorld) }
            if productionWorld.activeStage == .drawing || productionWorld.activeStage == .manufacture { ManufacturingHandoffView(appState: appState, store: productionWorld) }
            ProductionWorldView(appState: appState, store: productionWorld)
        }
    }

    private var navigationOverlay: some View {
        GeometryReader { geometry in
            NavigationSphereView(theta: cameraTheta, phi: cameraPhi, distance: cameraDistance)
                .position(x: geometry.size.width - 82, y: 82)
        }
    }

    private var radialMenuOverlay: some View {
        GeometryReader { geometry in
            if radialMenuVisible {
                let panels = RadialMenuGeometry.enabledPanels(radialMenuSettings.settings)
                let panelIndex = RadialMenuGeometry.panelIndex(for: radialVector.dx, dy: radialVector.dy, settings: radialMenuSettings.settings)
                let selectedPanel = panelIndex.flatMap { index in panels.indices.contains(index) ? panels[index] : nil }
                let selectedToolIndex = selectedPanel.flatMap { panel in RadialMenuGeometry.toolIndex(for: radialVector.dx, dy: radialVector.dy, panel: panel, settings: radialMenuSettings.settings) }
                let selectedTool: RadialMenuTool? = selectedPanel.flatMap { panel in
                    guard let index = selectedToolIndex, panel.tools.indices.contains(index) else { return nil }
                    return panel.tools[index]
                }
                VStack(spacing: 8) {
                    RadialMenuView(store: radialMenuSettings, center: CGPoint(x: radialCenterNormalized.x * geometry.size.width, y: radialCenterNormalized.y * geometry.size.height), vector: radialVector, onToolActivated: { tool in activateRadialTool(tool) }, onSettings: { radialSettingsPresented = true })
                    RadialMenuAvailabilityView(tool: selectedTool, context: appState.activeContext, policyStore: radialContextPolicies, registry: commandRegistry)
                }
                .position(x: radialCenterNormalized.x * geometry.size.width, y: radialCenterNormalized.y * geometry.size.height + 205)
            }
        }
        .allowsHitTesting(false)
    }

    private var notificationsOverlay: some View {
        VStack {
            HStack { Spacer(); NotificationsView(appState: appState).padding(.trailing, 18).padding(.top, 90) }
            Spacer()
        }
    }

    private var showEmptyState: Bool {
        appState.workbench == .model && modelRuntime.document.bodies.isEmpty
    }

    private var emptyState: some View {
        VStack(spacing: MirTheme.Spacing.md) {
            Image(systemName: "cube.transparent")
                .font(.system(size: 40, weight: .light))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text("МИР 4D")
                .font(.system(size: 20, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.textPrimary)
            Text(appState.ui.language == .russian ? "Проект пока пуст" : "Project is empty")
                .font(MirTheme.Typography.body)
                .foregroundStyle(MirTheme.Colors.textSecondary)
            Button {
                _ = commandRegistry.execute(id: "create.body", context: appState.activeContext)
            } label: {
                Label(appState.ui.language == .russian ? "Создать тело" : "Create Body", systemImage: "cube.transparent")
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.regular)
        }
        .padding(MirTheme.Spacing.xl)
        .background(MirTheme.Colors.surface.opacity(0.92))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.large))
        .overlay {
            RoundedRectangle(cornerRadius: MirTheme.Radius.large)
                .stroke(MirTheme.Colors.border, lineWidth: 1)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    private func userFacingErrorMessage(_ raw: String) -> String {
        let ru = appState.ui.language == .russian
        let lower = raw.lowercased()
        if lower.contains("unsupported") || lower.contains("format") {
            return ru ? "Не удалось импортировать модель: неподдерживаемый формат файла" : "Could not import model: unsupported file format"
        }
        if lower.contains("not found") || lower.contains("no such file") {
            return ru ? "Не удалось открыть файл: файл не найден" : "Could not open file: file not found"
        }
        if lower.contains("permission") || lower.contains("denied") {
            return ru ? "Не удалось получить доступ к файлу: недостаточно прав" : "Could not access file: permission denied"
        }
        if lower.contains("export") || lower.contains("write") {
            return ru ? "Не удалось экспортировать модель: ошибка записи" : "Could not export model: write error"
        }
        return ru ? "Не удалось выполнить операцию с моделью" : "Model operation failed"
    }

    private func applyContextualRadialLayout() {
        guard radialBasePanels == nil else { return }
        radialBasePanels = radialMenuSettings.settings.panels
        radialMenuSettings.settings.panels = RadialMenuContextLayout.panels(settings: radialMenuSettings.settings, context: radialContextStore.snapshot)
    }

    private func restoreBaseRadialLayout() {
        guard let basePanels = radialBasePanels else { return }
        radialMenuSettings.settings.panels = basePanels
        radialBasePanels = nil
    }

    private func activateRadialSelection(dx: Double, dy: Double) {
        let settings = radialMenuSettings.settings
        guard let panelIndex = RadialMenuGeometry.panelIndex(for: dx, dy: dy, settings: settings) else { return }
        let panels = RadialMenuGeometry.enabledPanels(settings)
        guard panels.indices.contains(panelIndex) else { return }
        let panel = panels[panelIndex]
        if let toolIndex = RadialMenuGeometry.toolIndex(for: dx, dy: dy, panel: panel, settings: settings), panel.tools.indices.contains(toolIndex) {
            activateRadialTool(panel.tools[toolIndex])
        } else {
            appState.showNotification(appState.ui.language == .russian ? "Радиальная панель: \(panel.title)" : "Radial panel: \(panel.title)", type: .info)
        }
    }

    @MainActor private func activateRadialTool(_ tool: RadialMenuTool) {
        switch tool.command {
        case "create.body": createBodyPresented = true
        case "file.import": presentMeshImportPanel()
        case "file.export": presentStlExportPanel(selectionOnly: false)
        default:
            if commandRegistry.execute(id: tool.command, context: appState.activeContext) { appState.showNotification(appState.ui.language == .russian ? "Инструмент: \(tool.title)" : "Tool: \(tool.title)", type: .success) }
            else { appState.showNotification(appState.ui.language == .russian ? "Команда недоступна: \(tool.title)" : "Command unavailable: \(tool.title)", type: .warning) }
        }
    }

    private func presentMeshImportPanel() {
        let panel = NSOpenPanel()
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false
        panel.canCreateDirectories = false
        panel.allowedContentTypes = mir4DImportTypes
        panel.title = appState.ui.language == .russian ? "Импорт модели" : "Import Model"
        panel.prompt = appState.ui.language == .russian ? "Импорт" : "Import"
        guard panel.runModal() == .OK, let url = panel.url else { return }
        NotificationCenter.default.post(name: .mir4DImportMesh, object: url.path)
        appState.showNotification(appState.ui.language == .russian ? "Импорт: \(url.lastPathComponent)" : "Importing: \(url.lastPathComponent)", type: .info)
    }

    private func presentStlExportPanel(selectionOnly: Bool) {
        let panel = NSSavePanel()
        panel.canCreateDirectories = true
        panel.allowedContentTypes = [mir4DSTLType]
        panel.nameFieldStringValue = selectionOnly ? "selected.stl" : "model.stl"
        panel.title = appState.ui.language == .russian ? (selectionOnly ? "Экспорт выбранного STL" : "Экспорт STL") : (selectionOnly ? "Export Selected STL" : "Export STL")
        panel.prompt = appState.ui.language == .russian ? "Экспорт" : "Export"
        guard panel.runModal() == .OK, let url = panel.url else { return }
        NotificationCenter.default.post(name: .mir4DExportStl, object: ["path": url.path, "selectionOnly": selectionOnly])
        appState.showNotification(appState.ui.language == .russian ? "Экспорт STL: \(url.lastPathComponent)" : "Export STL: \(url.lastPathComponent)", type: .info)
    }

    private var statusBar: some View {
        HStack(spacing: 7) {
            Circle().fill(MirTheme.Colors.success).frame(width: 6, height: 6)
            Text("МИР 4D").font(MirTheme.Typography.status).foregroundStyle(MirTheme.Colors.textSecondary)
            Text("•").foregroundStyle(MirTheme.Colors.textTertiary)
            Text(localizedWorkbench).font(MirTheme.Typography.status)
            Text("•").foregroundStyle(MirTheme.Colors.textTertiary)
            Text(localizedSubMode).font(MirTheme.Typography.status).foregroundStyle(MirTheme.Colors.textSecondary)
            Text("•").foregroundStyle(MirTheme.Colors.textTertiary)
            Text(appState.documentName).font(MirTheme.Typography.status).lineLimit(1)
            Spacer()
            if appState.selection.hasSelection { Text(selectionStatus).foregroundStyle(MirTheme.Colors.selection) }
            if appState.workbench == .sketch { Text(appState.interaction.snapEnabled ? "SNAP ON" : "SNAP OFF").foregroundStyle(appState.interaction.snapEnabled ? MirTheme.Colors.success : MirTheme.Colors.textTertiary) }
            if appState.workbench == .simulation { Text(simulationStatus).foregroundStyle(MirTheme.Colors.simulation) }
            if appState.workbench == .fourD { Text(String(format: "T %.3f s", appState.currentTime)).foregroundStyle(MirTheme.Colors.time) }
        }.padding(.horizontal, 12).padding(.vertical, 6).background(MirTheme.Colors.surface.opacity(0.9)).overlay(Rectangle().frame(height: 1).foregroundStyle(MirTheme.Colors.border), alignment: .top)
    }

    private var localizedWorkbench: String { appState.ui.language == .russian ? appState.workbench.titleRU : appState.workbench.titleEN }
    private var localizedSubMode: String { appState.ui.language == .russian ? appState.subMode.titleRU : appState.subMode.titleEN }
    private var selectionStatus: String { appState.ui.language == .russian ? "Выбрано: \(appState.selectionCount)" : "Selected: \(appState.selectionCount)" }
    private var simulationStatus: String { appState.simulation.solverStatus }
}

private struct PanelDropZone: View {
    let placement: PanelPlacement
    let target: PanelPlacement?
    let onTargetChange: @MainActor @Sendable (Bool) -> Void
    let onDropPanel: @MainActor @Sendable (PanelPlacement) -> Void

    var body: some View {
        let isTargeted = target == placement
        RoundedRectangle(cornerRadius: 10)
            .fill(isTargeted ? MirTheme.Colors.accent.opacity(0.14) : MirTheme.Colors.surface.opacity(0.55))
            .overlay {
                RoundedRectangle(cornerRadius: 10)
                    .stroke(isTargeted ? MirTheme.Colors.accentBright : MirTheme.Colors.borderStrong, lineWidth: isTargeted ? 2 : 1)
            }
            .overlay {
                VStack(spacing: 6) {
                    Image(systemName: placement.icon).font(.system(size: 14))
                    Text(placement.titleRU).font(.system(size: 10, weight: .semibold))
                }
                .foregroundStyle(isTargeted ? MirTheme.Colors.accentBright : MirTheme.Colors.textSecondary)
            }
            .contentShape(Rectangle())
            .onDrop(of: [UTType.text], isTargeted: Binding(get: { target == placement }, set: { isTargeted in Task { @MainActor in onTargetChange(isTargeted) } })) { providers in
                guard let provider = providers.first else { return false }
                provider.loadObject(ofClass: NSString.self) { object, _ in
                    guard let raw = object as? String, CADPanel(rawValue: raw) != nil else { return }
                    Task { @MainActor in onDropPanel(placement) }
                }
                return true
            }
    }
}
