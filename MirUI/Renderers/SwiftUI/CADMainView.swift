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
                        if resizeBaseline[edge] == nil {
                            resizeBaseline[edge] = currentDimension(for: edge)
                        }
                        let baseline = resizeBaseline[edge] ?? currentDimension(for: edge)
                        switch edge {
                        case .left:
                            workspace.leftWidth = min(420, max(180, baseline + value.translation.width))
                        case .right:
                            workspace.rightWidth = min(480, max(220, baseline - value.translation.width))
                        case .bottom:
                            workspace.bottomHeight = min(420, max(140, baseline - value.translation.height))
                        }
                    }
                    .onEnded { _ in resizeBaseline.removeValue(forKey: edge) }
            )
            .help(edge == .bottom ? "Изменить высоту панели" : "Изменить ширину панели")
    }

    private func currentDimension(for edge: ResizeEdge) -> Double {
        switch edge {
        case .left: return workspace.leftWidth
        case .right: return workspace.rightWidth
        case .bottom: return workspace.bottomHeight
        }
    }

    private func dockedPanels(_ placement: PanelPlacement) -> [CADPanel] {
        appState.visiblePanels.filter { appState.panelPlacement(for: $0) == placement }.sorted { $0.rawValue < $1.rawValue }
    }

    private var centerColumn: some View {
        ZStack {
            viewport
            viewportOverlay
            CADViewportChrome(appState: appState, cameraTheta: $cameraTheta, cameraPhi: $cameraPhi, cameraDistance: $cameraDistance, isOrthographic: $isOrthographic)
            navigationOverlay
            emptyStateHint
            if appState.workbench == .sketch {
                sketchLayer
                if appState.sketchPlane == nil {
                    SketchPlaneChooser(onPick: { appState.sketchPlane = $0 })
                }
            }
        }
        .background(MirTheme.Colors.viewport)
        .clipped()
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    private var sketchLayer: some View {
        SketchViewportView()
            .environmentObject(appState)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .allowsHitTesting(true)
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
        NavigationSphereView(
            theta: cameraTheta,
            phi: cameraPhi,
            distance: cameraDistance,
            isOrthographic: isOrthographic
        )
        // 35% прозрачность: куб не должен перекрывать обзор модели.
        .opacity(0.65)
        // На 30% меньше базового размера, затем ещё на 20% (итого 0.56).
        .scaleEffect(0.56)
        .fixedSize()
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
        // Отступы от краёв по 5 мм (14 pt).
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
                    Button { createDefaultBox() } label: { Label(appState.ui.language == .russian ? "Новое тело" : "New Body", systemImage: "cube.transparent") }
                        .buttonStyle(.borderedProminent).controlSize(.small).tint(MirTheme.Colors.accent)
                    Button { showImporter = true } label: { Label(appState.ui.language == .russian ? "Импорт" : "Import", systemImage: "square.and.arrow.down") }
                        .buttonStyle(.bordered).controlSize(.small)
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
        // ТЗ: показываем цветные рабочие плоскости только в режиме эскиза.
        view.syncWorkPlanesIfNeeded()
    }
}

struct CADViewportChrome: View {
    @ObservedObject var appState: CADAppState
    @Binding var cameraTheta: Double
    @Binding var cameraPhi: Double
    @Binding var cameraDistance: Double
    @Binding var isOrthographic: Bool

    var body: some View { ZStack { viewportBadge; viewportControls; viewportReadout } }

    private var viewportBadge: some View {
        HStack(spacing: 7) {
            Image(systemName: "cube.transparent").font(.system(size: 11, weight: .semibold)).foregroundStyle(MirTheme.Colors.accentBright)
            Text(appState.ui.language == .russian ? "3D ВИД" : "3D VIEW").font(.system(size: 10, weight: .semibold)).tracking(0.6).foregroundStyle(MirTheme.Colors.textSecondary)
        }
        .padding(.horizontal, 10).padding(.vertical, 7).background(MirTheme.Colors.surfaceRaised).clipShape(Capsule())
        .overlay(Capsule().stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 1))
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading).padding(10).allowsHitTesting(false)
    }

    private var viewportControls: some View {
        Group {
            if appState.workbench == .sketch {
                // Режим эскиза имеет собственную панель инструментов.
                EmptyView()
            } else {
                VStack(spacing: 6) {
                    workbenchToolGroup
                }
                .padding(8)
                .background(MirTheme.Colors.surfaceRaised.opacity(0.92), in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
                .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.medium).stroke(MirTheme.Colors.panelBorder, lineWidth: 1))
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .leading)
                .padding(10)
            }
        }
    }

    /// Инструменты левой вертикальной панели: набор зависит от текущего
    /// режима работы (workbench).
    @ViewBuilder
    private var workbenchToolGroup: some View {
        switch appState.workbench {
        case .model:
            baseToolGroup
            viewportButton("cube.transparent", "Новое тело") { _ = MIR4DModelCommands.shared.createBox(appState: appState, width: 40, depth: 40, height: 40) }
            viewportButton("square.2.layers.3d", "Рабочая плоскость (XY +10)") { MIR4DModelCommands.shared.createWorkPlane(basePlane: 1, offset: 10.0) }
            viewportButton("pencil.tip", "Эскиз: прямоугольник (ограничения)") { MIR4DModelCommands.shared.createSketchRectangle(appState: appState) }
        case .assembly:
            baseToolGroup
            viewportButton("link", "Связать детали") { MirEventBus.shared.publish(.commandRequested("assembly.mate")) }
            viewportButton("exclamationmark.triangle", "Проверить пересечения") { MirEventBus.shared.publish(.commandRequested("assembly.interference")) }
        case .simulation:
            baseToolGroup
            viewportButton("play.circle", "Запустить расчёт") {
                MirEventBus.shared.publish(.commandRequested("simulation.solve"))
                appState.runSimulation()
            }
            viewportButton("chart.xyaxis.line", "Показать результаты") {
                appState.simulation.phase = .results
                MirEventBus.shared.publish(.commandRequested("simulation.results"))
            }
        case .fourD:
            baseToolGroup
            viewportButton("play.fill", "Воспроизвести время") { appState.togglePlayback() }
            viewportButton("arrow.triangle.branch", "Создать ветку сценария") { appState.createTimeBranch(); MirEventBus.shared.publish(.commandRequested("fourD.branch")) }
            viewportButton("square.split.2x1", "Сравнить состояния") { appState.subMode = .fourDCompare; MirEventBus.shared.publish(.commandRequested("fourD.compare")) }
            viewportButton("questionmark.diamond", "Что если") { appState.subMode = .fourDWhatIf; MirEventBus.shared.publish(.commandRequested("fourD.whatIf")) }
        case .drawing:
            baseToolGroup
            viewportButton("ruler", "Измерение") { appState.selectedTool = "measure" }
            viewportButton("grid", "Сетка") { appState.toggleGrid() }
        case .collaboration, .visualization:
            baseToolGroup
        case .sketch:
            EmptyView()
        }
    }

    private var baseToolGroup: some View {
        VStack(spacing: 6) {
            viewportButton("arrow.up.left.and.arrow.down.right", "Подогнать модель") { NotificationCenter.default.post(name: .mir4DFitViewport, object: nil) }
            viewportButton("viewfinder", "Центрировать") { NotificationCenter.default.post(name: .mir4DFitViewport, object: nil) }
            viewportButton(isOrthographic ? "perspective" : "rectangle.on.rectangle", isOrthographic ? "Включить перспективу" : "Включить ортографическую проекцию") {
                isOrthographic.toggle()
                NotificationCenter.default.post(name: .mir4DCameraProjectionRequested, object: nil, userInfo: ["projection": isOrthographic ? 1 : 0])
            }
        }
    }

    private func viewportButton(_ image: String, _ help: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: image).font(.system(size: 11, weight: .medium)).frame(width: 26, height: 26).foregroundStyle(MirTheme.Colors.textSecondary).background(MirTheme.Colors.surface).clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain).help(help)
    }

    private var viewportReadout: some View {
        VStack {
            Spacer()
            HStack(spacing: 10) {
                Label("X\(format(cameraTheta))", systemImage: "arrow.left.and.right")
                Label("Y\(format(cameraPhi))", systemImage: "arrow.up.and.down")
                Label("D\(format(cameraDistance))", systemImage: "ruler")
                Spacer()
                Text(appState.ui.language == .russian ? "\(isOrthographic ? "Ортографическая" : "Перспектива") · Колесо — масштаб · ПКМ — панорама" : "\(isOrthographic ? "Orthographic" : "Perspective") · Wheel — zoom · RMB — pan")
                    .font(.system(size: 9)).foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .font(.system(size: 9, weight: .medium, design: .monospaced)).foregroundStyle(MirTheme.Colors.textSecondary)
            .padding(.horizontal, 10).padding(.vertical, 6).background(MirTheme.Colors.surfaceRaised).clipShape(Capsule())
            .overlay(Capsule().stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 1)).padding(10)
        }
        .allowsHitTesting(false)
    }

    private func format(_ value: Double) -> String { String(format: "%.1f", value) }
}
