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

    var body: some View {
        ZStack {
            appearance.theme.windowBackground.ignoresSafeArea()
            VStack(spacing: 0) {
                TopBarView(appState: appState, registry: commandRegistry, commandPalettePresented: $commandPalettePresented)
                HStack(spacing: 0) {
                    if appState.visiblePanels.contains(.project) {
                        SidebarView(appState: appState).frame(minWidth: 240, idealWidth: 280, maxWidth: 340)
                    }
                    viewport.frame(maxWidth: .infinity, maxHeight: .infinity)
                    if appState.visiblePanels.contains(.properties) {
                        VStack(spacing: 0) {
                            SelectionIdentityInspector(appState: appState)
                            InspectorTabsView(appState: appState)
                        }.frame(minWidth: 280, idealWidth: 320, maxWidth: 380)
                    }
                }
                .animation(MirTheme.Animation.normal, value: appState.visiblePanels)
                if appState.visiblePanels.contains(.timeline) { timeline }
            }
            VStack {
                HStack { Spacer(); NotificationsView(appState: appState).padding(.trailing, 18).padding(.top, 90) }
                Spacer()
            }
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
            appState.showNotification(appState.ui.language == .russian ? "МИР 4D готов · интерфейсный слой активирован" : "MIR 4D ready · interface layer activated", type: .success)
        }
    }

    private var viewport: some View {
        ZStack {
            MirGLView(
                onSelectionChanged: { objectId in
                    if objectId == 0 { appState.clearSelection() } else { appState.setSelection(ids: [String(objectId)], kind: .body) }
                },
                onIOError: { message in appState.showNotification(message, type: .error) },
                onCameraOrientationChanged: { theta, phi, distance in cameraTheta = theta; cameraPhi = phi; cameraDistance = distance }
            )
            .background(MirTheme.Colors.viewport)

            VStack(spacing: 0) {
                HStack(alignment: .top, spacing: 0) {
                    VStack(alignment: .leading, spacing: 8) {
                        HStack(spacing: 8) {
                            SelectionFilterBar(appState: appState)
                            Button { createBodyPresented = true } label: { Label(appState.ui.language == .russian ? "Новое тело" : "New Body", systemImage: "cube.transparent") }.buttonStyle(.borderedProminent).controlSize(.small)
                            Button { presentMeshImportPanel() } label: { Label(appState.ui.language == .russian ? "Импорт" : "Import", systemImage: "square.and.arrow.down") }.buttonStyle(.bordered).controlSize(.small)
                            Button { presentStlExportPanel(selectionOnly: false) } label: { Label(appState.ui.language == .russian ? "Экспорт STL" : "Export STL", systemImage: "square.and.arrow.up") }.buttonStyle(.bordered).controlSize(.small)
                            if appState.selection.hasSelection {
                                Button { presentStlExportPanel(selectionOnly: true) } label: { Label(appState.ui.language == .russian ? "Выбранное" : "Selected", systemImage: "arrow.up.doc") }.buttonStyle(.bordered).controlSize(.small)
                            }
                            Button { radialSettingsPresented = true } label: { Image(systemName: "circle.grid.3x3.fill") }.buttonStyle(.bordered).controlSize(.small)
                        }.padding(.top, 10).padding(.leading, 12)
                        ContextualToolbarView(appState: appState, registry: commandRegistry).padding(.leading, 12)
                        WorkbenchContentRouter(appState: appState, registry: commandRegistry)
                    }
                    Spacer()
                    if appState.workbench == .fourD || appState.workbench == .simulation { FourDSceneOverlayView(appState: appState) }
                }
                Spacer()
                if productionWorld.activeStage == .idea { IdeaStudioView(appState: appState, store: productionWorld) }
                if productionWorld.activeStage == .test || productionWorld.activeStage == .scenario { DigitalWorldHUD(appState: appState, store: productionWorld) }
                if productionWorld.activeStage == .drawing || productionWorld.activeStage == .manufacture { ManufacturingHandoffView(appState: appState, store: productionWorld) }
                ProductionWorldView(appState: appState, store: productionWorld)
                statusBar
            }

            GeometryReader { geometry in
                NavigationSphereView(theta: cameraTheta, phi: cameraPhi, distance: cameraDistance).position(x: geometry.size.width - 92, y: 92)
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
            }.allowsHitTesting(false)
        }
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

    @ViewBuilder private var timeline: some View {
        switch appState.workbench {
        case .fourD, .simulation: FourDTimelineView(appState: appState).frame(minHeight: 170, idealHeight: 230, maxHeight: 300)
        case .assembly: TimelinePanelView(appState: appState).frame(minHeight: 170, idealHeight: 220, maxHeight: 280)
        default: TimelinePanelView(appState: appState).frame(minHeight: 190, idealHeight: 250, maxHeight: 320)
        }
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
