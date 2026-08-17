import SwiftUI
import AppKit
import UniformTypeIdentifiers

private let mir4DImportTypes: [UTType] = [
    UTType(filenameExtension: "stl", conformingTo: .data),
    UTType(filenameExtension: "obj", conformingTo: .data),
    UTType(filenameExtension: "ply", conformingTo: .data),
    UTType(filenameExtension: "gltf", conformingTo: .data),
    UTType(filenameExtension: "glb", conformingTo: .data),
    UTType(filenameExtension: "fbx", conformingTo: .data),
    UTType(filenameExtension: "step", conformingTo: .data),
    UTType(filenameExtension: "stp", conformingTo: .data)
].compactMap { $0 }

struct CADMainView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var workspace = MIR4DWorkspaceCustomizationStore.shared
    @StateObject private var registry = CADCommandRegistry()
    @StateObject private var productionStore = ProductionWorldStore()

    @State private var commandPalettePresented = false
    @State private var radialSettingsPresented = false
    @State private var showImporter = false
    @State private var cameraTheta: Double = 0
    @State private var cameraPhi: Double = 0
    @State private var cameraDistance: Double = 1
    @State private var isOrthographic: Bool = false
    @State private var showEmptyState = true
    @State private var viewportSize: CGSize = .zero
    @State private var radialCenter: CGPoint = .zero
    @State private var radialVector: CGVector = .zero
    @State private var radialMenuPresented = false
    @State private var resizeBaseline: [ResizeEdge: Double] = [:]

    var body: some View {
        ZStack {
            mainLayout
            if radialMenuPresented { radialMenuOverlay }
        }
        .background(MirTheme.Colors.background)
        .sheet(isPresented: $commandPalettePresented) { CommandPaletteView(appState: appState, registry: registry) }
        .sheet(isPresented: $radialSettingsPresented) { RadialMenuSettingsView(store: RadialMenuSettingsStore.shared) }
        .fileImporter(isPresented: $showImporter, allowedContentTypes: mir4DImportTypes, allowsMultipleSelection: false) { result in
            guard case .success(let urls) = result, let url = urls.first else { return }
            NotificationCenter.default.post(name: .mir4DImportMesh, object: url.path)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuBegan)) { note in
            guard let x = note.userInfo?["x"] as? Double, let y = note.userInfo?["y"] as? Double else { return }
            radialCenter = CGPoint(x: x * viewportSize.width, y: y * viewportSize.height)
            radialVector = .zero
            radialMenuPresented = true
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuMoved)) { note in
            guard radialMenuPresented, let dx = note.userInfo?["dx"] as? Double, let dy = note.userInfo?["dy"] as? Double else { return }
            radialVector = CGVector(dx: dx, dy: dy)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuEnded)) { _ in
            radialMenuPresented = false
            radialVector = .zero
        }
        .onChange(of: appState.panelState) { _, _ in FloatingPanelManager.shared.sync(appState: appState) }
        .onAppear {
            print("TRACE: CADMainView onAppear")
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
            FloatingPanelManager.shared.sync(appState: appState)
        }
        .onDisappear { FloatingPanelManager.shared.closeAll() }
    }

    private var mainLayout: some View {
        VStack(spacing: 0) {
            TopBarView(appState: appState, registry: registry, commandPalettePresented: $commandPalettePresented)
            HStack(spacing: 0) {
                leftPanelColumn
                centerColumn
                rightPanelColumn
            }
            bottomPanelRow
        }
    }

    private var leftPanelColumn: some View {
        HStack(spacing: 0) {
            VStack(spacing: 0) {
                ForEach(dockedPanels(.left)) { panel in CADPanelView(panel: panel, appState: appState).frame(maxWidth: .infinity, maxHeight: .infinity) }
            }
            .background(MirTheme.Colors.panel)
            .frame(width: workspace.leftWidth)
            .overlay(alignment: .trailing) { resizeHandle(edge: .left) }
        }
    }

    private var rightPanelColumn: some View {
        HStack(spacing: 0) {
            resizeHandle(edge: .right)
            VStack(spacing: 0) {
                ForEach(dockedPanels(.right)) { panel in CADPanelView(panel: panel, appState: appState).frame(maxWidth: .infinity, maxHeight: .infinity) }
            }
            .background(MirTheme.Colors.panel)
            .frame(width: workspace.rightWidth)
        }
    }

    @ViewBuilder
    private var bottomPanelRow: some View {
        let panels = dockedPanels(.bottom)
        if !panels.isEmpty {
            VStack(spacing: 0) {
                resizeHandle(edge: .bottom)
                HStack(spacing: 0) {
                    ForEach(panels) { panel in CADPanelView(panel: panel, appState: appState).frame(maxWidth: .infinity, maxHeight: .infinity) }
                }
            }
            .frame(height: workspace.bottomHeight)
            .background(MirTheme.Colors.panel)
        }
    }

    private enum ResizeEdge: Hashable { case left, right, bottom }

    private func resizeHandle(edge: ResizeEdge) -> some View {
        Rectangle()
            .fill(MirTheme.Colors.border.opacity(0.8))
            .frame(width: edge == .bottom ? nil : 5, height: edge == .bottom ? 5 : nil)
            .contentShape(Rectangle())
            .gesture(
                DragGesture(minimumDistance: 1)
                    .onChanged { value in
                        if resizeBaseline[edge] == nil { resizeBaseline[edge] = currentDimension(for: edge) }
                        let baseline = resizeBaseline[edge] ?? currentDimension(for: edge)
                        switch edge {
                        case .left: workspace.leftWidth = min(420, max(180, baseline + value.translation.width))
                        case .right: workspace.rightWidth = min(480, max(220, baseline - value.translation.width))
                        case .bottom: workspace.bottomHeight = min(420, max(140, baseline - value.translation.height))
                        }
                    }
                    .onEnded { _ in resizeBaseline.removeValue(forKey: edge) }
            )
            .help(edge == .bottom ? "Изменить высоту панели" : "Изменить ширину панели")
    }

    private func currentDimension(for edge: ResizeEdge) -> Double {
        switch edge { case .left: return workspace.leftWidth; case .right: return workspace.rightWidth; case .bottom: return workspace.bottomHeight }
    }

    private func dockedPanels(_ placement: PanelPlacement) -> [CADPanel] {
        appState.visiblePanels.filter { appState.panelPlacement(for: $0) == placement }.sorted { $0.rawValue < $1.rawValue }
    }

    private var centerColumn: some View {
        ZStack {
            viewport
            viewportOverlay
            navigationOverlay
            emptyStateHint
            if appState.selection.hasSelection {
                ContextualToolbarView(
                    appState: appState,
                    registry: registry,
                    onCommandPalette: { commandPalettePresented = true }
                )
                .padding(.horizontal, 18)
                .padding(.bottom, 14)
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .bottom)
                .transition(.move(edge: .bottom).combined(with: .opacity))
            }
            if appState.workbench == .sketch {
                sketchLayer
                if appState.sketchPlane == nil { SketchPlaneChooser(onPick: { appState.sketchPlane = $0 }) }
            }
        }
        .background(MirTheme.Colors.viewport)
        .clipped()
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .animation(.easeOut(duration: 0.16), value: appState.selection.hasSelection)
    }

    private var sketchLayer: some View {
        SketchViewportView().environmentObject(appState).frame(maxWidth: .infinity, maxHeight: .infinity).allowsHitTesting(true)
    }

    private var viewport: some View {
        GeometryReader { proxy in
            ViewportRepresentable(
                appState: appState,
                onSelectionChanged: { objectID in appState.setSelection(ids: objectID > 0 ? ["\(objectID)"] : [], kind: objectID > 0 ? .body : .none) },
                onIOError: { message in appState.showNotification(message, type: .error) },
                onCameraOrientationChanged: { theta, phi, distance in
                    DispatchQueue.main.async {
                        cameraTheta = theta
                        cameraPhi = phi
                        cameraDistance = distance
                        showEmptyState = false
                    }
                }
            )
            .onAppear { viewportSize = proxy.size }
            .onChange(of: proxy.size) { _, newSize in viewportSize = newSize }
        }
    }

    private var viewportOverlay: some View {
        ZStack {
            FourDSceneOverlayView(appState: appState)
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
                .padding(.top, 44)
                .allowsHitTesting(false)
            if appState.workbench == .fourD {
                DigitalWorldHUD(appState: appState, store: productionStore)
                ProductionWorldView(appState: appState, store: productionStore).frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .bottom)
            }
        }
    }

    private var navigationOverlay: some View {
        NavigationSphereView(theta: cameraTheta, phi: cameraPhi, distance: cameraDistance, isOrthographic: isOrthographic)
            .opacity(0.65)
            .scaleEffect(0.56)
            .fixedSize()
            .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
            .padding(.top, 14)
            .padding(.trailing, 14)
    }

    @ViewBuilder
    private var emptyStateHint: some View {
        if showEmptyState, appState.treeData.first?.children.isEmpty ?? true {
            VStack(spacing: 16) {
                Image(systemName: "cube.transparent").font(.system(size: 42, weight: .light)).foregroundStyle(MirTheme.Colors.textTertiary)
                Text(appState.ui.language == .russian ? "Рабочая область свободна" : "Workspace is empty").font(MirTheme.Typography.title).foregroundStyle(MirTheme.Colors.textSecondary)
                Text(appState.ui.language == .russian ? "Создайте первый инженерный объект или импортируйте модель." : "Create your first engineering object or import a model.").font(MirTheme.Typography.caption).foregroundStyle(MirTheme.Colors.textTertiary)
                HStack(spacing: 10) {
                    Button { createDefaultBox() } label: { Label(appState.ui.language == .russian ? "Новое тело" : "New Body", systemImage: "cube.transparent") }.buttonStyle(.borderedProminent).controlSize(.small).tint(MirTheme.Colors.accent)
                    Button { showImporter = true } label: { Label(appState.ui.language == .russian ? "Импорт" : "Import", systemImage: "square.and.arrow.down") }.buttonStyle(.bordered).controlSize(.small)
                }
            }
            .padding(28)
            .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: MirTheme.Radius.floating))
            .overlay { RoundedRectangle(cornerRadius: MirTheme.Radius.floating).stroke(MirTheme.Colors.panelBorder.opacity(0.8), lineWidth: 1) }
            .shadow(color: .black.opacity(0.25), radius: 18, y: 8)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
    }

    private var radialMenuOverlay: some View {
        RadialMenuView(store: RadialMenuSettingsStore.shared, center: radialCenter, vector: radialVector, onToolActivated: { _ in radialMenuPresented = false }, onSettings: { radialSettingsPresented = true })
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(Color.black.opacity(0.10).allowsHitTesting(false))
    }

    private func createDefaultBox() { _ = MIR4DModelCommands.shared.createBox(appState: appState, width: 40, depth: 40, height: 40) }
}

private struct ViewportRepresentable: NSViewRepresentable {
    var appState: CADAppState
    var onSelectionChanged: (UInt64) -> Void
    var onIOError: (String) -> Void
    var onCameraOrientationChanged: (Double, Double, Double) -> Void

    func makeNSView(context: Context) -> MirGLCustomView {
        let view = MirGLCustomView()
        apply(to: view)
        return view
    }

    func updateNSView(_ nsView: MirGLCustomView, context: Context) { apply(to: nsView) }

    private func apply(to view: MirGLCustomView) {
        view.appState = appState
        view.onSelectionChanged = onSelectionChanged
        view.onIOError = onIOError
        view.onCameraOrientationChanged = onCameraOrientationChanged
        view.syncWorkPlanesIfNeeded()
    }
}

struct CADViewportChrome: View {
    @ObservedObject var appState: CADAppState
    @Binding var cameraTheta: Double
    @Binding var cameraPhi: Double
    @Binding var cameraDistance: Double
    @Binding var isOrthographic: Bool

    /// Legacy compatibility shell. Viewport actions are now contextual and
    /// rendered by CADMainView through ContextualToolbarView. Keeping this
    /// type avoids breaking external references while removing the old
    /// permanent chrome from the viewport.
    var body: some View { EmptyView() }
}
