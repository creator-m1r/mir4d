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

// CADMainView is the presentation root of the CAD workspace.
// MirEngine remains the owner of geometry, camera state and engineering data;
// this view only composes existing modules: top bar, panels, viewport chrome,
// navigation sphere, radial menu mirror and 4D overlays.

struct CADMainView: View {
    @ObservedObject var appState: CADAppState
    @StateObject private var registry = CADCommandRegistry()
    @StateObject private var productionStore = ProductionWorldStore()

    @State private var commandPalettePresented = false
    @State private var radialSettingsPresented = false
    @State private var showImporter = false

    // Camera state is owned by MirEngine; the UI only mirrors it for the readout.
    @State private var cameraTheta: Double = 0
    @State private var cameraPhi: Double = 0
    @State private var cameraDistance: Double = 1
    @State private var showEmptyState = true
    @State private var viewportSize: CGSize = .zero

    // Radial menu overlay mirrors MirGLView gesture notifications.
    @State private var radialCenter: CGPoint = .zero
    @State private var radialVector: CGVector = .zero
    @State private var radialMenuPresented = false

    var body: some View {
        ZStack {
            mainLayout

            if radialMenuPresented {
                radialMenuOverlay
            }
        }
        .background(MirTheme.Colors.background)
        .sheet(isPresented: $commandPalettePresented) {
            CommandPaletteView(appState: appState, registry: registry)
        }
        .sheet(isPresented: $radialSettingsPresented) {
            RadialMenuSettingsView(store: RadialMenuSettingsStore.shared)
        }
        .fileImporter(
            isPresented: $showImporter,
            allowedContentTypes: mir4DImportTypes,
            allowsMultipleSelection: false
        ) { result in
            guard case .success(let urls) = result, let url = urls.first else { return }
            NotificationCenter.default.post(name: .mir4DImportMesh, object: url.path)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuBegan)) { note in
            guard
                let x = note.userInfo?["x"] as? Double,
                let y = note.userInfo?["y"] as? Double
            else { return }
            radialCenter = CGPoint(x: x * viewportSize.width, y: y * viewportSize.height)
            radialVector = .zero
            radialMenuPresented = true
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuMoved)) { note in
            guard
                radialMenuPresented,
                let dx = note.userInfo?["dx"] as? Double,
                let dy = note.userInfo?["dy"] as? Double
            else { return }
            radialVector = CGVector(dx: dx, dy: dy)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuEnded)) { _ in
            radialMenuPresented = false
            radialVector = .zero
        }
        .onChange(of: appState.panelState) { _, _ in
            FloatingPanelManager.shared.sync(appState: appState)
        }
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
            FloatingPanelManager.shared.sync(appState: appState)
        }
        .onDisappear {
            FloatingPanelManager.shared.closeAll()
        }
    }

    // MARK: - Layout

    private var mainLayout: some View {
        VStack(spacing: 0) {
            TopBarView(
                appState: appState,
                registry: registry,
                commandPalettePresented: $commandPalettePresented
            )
            HStack(spacing: 0) {
                leftPanelColumn
                centerColumn
                rightPanelColumn
            }
            bottomPanelRow
        }
    }

    private var leftPanelColumn: some View {
        VStack(spacing: 0) {
            ForEach(dockedPanels(.left)) { panel in
                CADPanelView(panel: panel, appState: appState)
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
        }
        .frame(width: 236)
        .background(MirTheme.Colors.panel)
        .overlay(alignment: .trailing) {
            Rectangle().fill(MirTheme.Colors.border).frame(width: 1)
        }
    }

    private var rightPanelColumn: some View {
        VStack(spacing: 0) {
            ForEach(dockedPanels(.right)) { panel in
                CADPanelView(panel: panel, appState: appState)
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
        }
        .frame(width: 280)
        .background(MirTheme.Colors.panel)
        .overlay(alignment: .leading) {
            Rectangle().fill(MirTheme.Colors.border).frame(width: 1)
        }
    }

    @ViewBuilder
    private var bottomPanelRow: some View {
        let panels = dockedPanels(.bottom)
        if !panels.isEmpty {
            HStack(spacing: 0) {
                ForEach(panels) { panel in
                    CADPanelView(panel: panel, appState: appState)
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                }
            }
            .frame(height: 240)
            .background(MirTheme.Colors.panel)
            .overlay(alignment: .top) {
                Rectangle().fill(MirTheme.Colors.border).frame(height: 1)
            }
        }
    }

    private func dockedPanels(_ placement: PanelPlacement) -> [CADPanel] {
        appState.visiblePanels
            .filter { appState.panelPlacement(for: $0) == placement }
            .sorted { $0.rawValue < $1.rawValue }
    }

    // MARK: - Center column

    private var centerColumn: some View {
        ZStack {
            viewport
            viewportOverlay
            CADViewportChrome(
                appState: appState,
                cameraTheta: $cameraTheta,
                cameraPhi: $cameraPhi,
                cameraDistance: $cameraDistance
            )
            navigationOverlay
            emptyStateHint
        }
        .background(MirTheme.Colors.viewport)
        .clipped()
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    private var viewport: some View {
        GeometryReader { proxy in
            ViewportRepresentable(
                onSelectionChanged: { objectID in
                    if objectID > 0 {
                        appState.setSelection(ids: ["\(objectID)"], kind: .body)
                    } else {
                        appState.setSelection(ids: [], kind: .none)
                    }
                },
                onIOError: { message in
                    appState.showNotification(message, type: .error)
                },
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
            .onChange(of: proxy.size) { _, newSize in
                viewportSize = newSize
            }
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
                ProductionWorldView(appState: appState, store: productionStore)
                    .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .bottom)
            }
        }
    }

    private var navigationOverlay: some View {
        NavigationSphereView(
            theta: cameraTheta,
            phi: cameraPhi,
            distance: cameraDistance
        )
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
        .padding(.top, 92)
        .padding(.trailing, 10)
    }

    @ViewBuilder
    private var emptyStateHint: some View {
        if showEmptyState, appState.treeData.first?.children.isEmpty ?? true {
            VStack(spacing: 16) {
                Image(systemName: "cube.transparent")
                    .font(.system(size: 42, weight: .light))
                    .foregroundStyle(MirTheme.Colors.textTertiary)

                Text(appState.ui.language == .russian ? "Рабочая область свободна" : "Workspace is empty")
                    .font(MirTheme.Typography.title)
                    .foregroundStyle(MirTheme.Colors.textSecondary)

                Text(
                    appState.ui.language == .russian
                        ? "Создайте первый инженерный объект или импортируйте модель."
                        : "Create your first engineering object or import a model."
                )
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textTertiary)

                HStack(spacing: 10) {
                    Button {
                        createDefaultBox()
                    } label: {
                        Label(
                            appState.ui.language == .russian ? "Новое тело" : "New Body",
                            systemImage: "cube.transparent"
                        )
                    }
                    .buttonStyle(.borderedProminent)
                    .controlSize(.small)
                    .tint(MirTheme.Colors.accent)

                    Button {
                        showImporter = true
                    } label: {
                        Label(
                            appState.ui.language == .russian ? "Импорт" : "Import",
                            systemImage: "square.and.arrow.down"
                        )
                    }
                    .buttonStyle(.bordered)
                    .controlSize(.small)
                }
            }
            .padding(28)
            .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: MirTheme.Radius.floating))
            .overlay {
                RoundedRectangle(cornerRadius: MirTheme.Radius.floating)
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.8), lineWidth: 1)
            }
            .shadow(color: .black.opacity(0.25), radius: 18, y: 8)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
    }

    private var radialMenuOverlay: some View {
        RadialMenuView(
            store: RadialMenuSettingsStore.shared,
            center: radialCenter,
            vector: radialVector,
            onToolActivated: { _ in
                radialMenuPresented = false
            },
            onSettings: {
                radialSettingsPresented = true
            }
        )
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color.black.opacity(0.10).allowsHitTesting(false))
    }

    private func createDefaultBox() {
        _ = MIR4DModelCommands.shared.createBox(
            appState: appState,
            width: 40,
            depth: 40,
            height: 40
        )
    }
}

// MARK: - MirGLView bridge

private struct ViewportRepresentable: NSViewRepresentable {
    var onSelectionChanged: (UInt64) -> Void
    var onIOError: (String) -> Void
    var onCameraOrientationChanged: (Double, Double, Double) -> Void

    func makeNSView(context: Context) -> MirGLCustomView {
        let view = MirGLCustomView()
        apply(to: view)
        return view
    }

    func updateNSView(_ nsView: MirGLCustomView, context: Context) {
        apply(to: nsView)
    }

    private func apply(to view: MirGLCustomView) {
        view.onSelectionChanged = onSelectionChanged
        view.onIOError = onIOError
        view.onCameraOrientationChanged = onCameraOrientationChanged
    }
}

// MARK: - Viewport chrome

// CADViewportChrome is the overlay-only viewport decoration:
// 3D badge, quick controls and the camera readout.
// It never paints an opaque background — the MirGLView renders beneath it.

struct CADViewportChrome: View {
    @ObservedObject var appState: CADAppState
    @Binding var cameraTheta: Double
    @Binding var cameraPhi: Double
    @Binding var cameraDistance: Double
    @State private var isOrthographic: Bool = false

    var body: some View {
        ZStack {
            viewportBadge
            viewportControls
            viewportReadout
        }
    }

    private var viewportBadge: some View {
        HStack(spacing: 7) {
            Image(systemName: "cube.transparent")
                .font(.system(size: 11, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)
            Text(appState.ui.language == .russian ? "3D ВИД" : "3D VIEW")
                .font(.system(size: 10, weight: .semibold))
                .tracking(0.6)
                .foregroundStyle(MirTheme.Colors.textSecondary)
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 7)
        .background(MirTheme.Colors.surfaceRaised)
        .clipShape(Capsule())
        .overlay(Capsule().stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 1))
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
        .padding(10)
        .allowsHitTesting(false)
    }

    private var viewportControls: some View {
        VStack(spacing: 6) {
            viewportButton("arrow.up.left.and.arrow.down.right", "Подогнать модель") {
                NotificationCenter.default.post(name: .mir4DFitViewport, object: nil)
            }
            viewportButton("viewfinder", "Центрировать") {
                NotificationCenter.default.post(name: .mir4DFitViewport, object: nil)
            }
            viewportButton(
                isOrthographic ? "perspective" : "rectangle.on.rectangle",
                isOrthographic
                    ? "Включить перспективу"
                    : "Включить ортографическую проекцию"
            ) {
                isOrthographic.toggle()
                NotificationCenter.default.post(
                    name: .mir4DCameraProjectionRequested,
                    object: nil,
                    userInfo: [
                        "projection": isOrthographic ? 1 : 0
                    ]
                )
            }
            viewportButton("cube.transparent", "Новое тело") {
                _ = MIR4DModelCommands.shared.createBox(
                    appState: appState,
                    width: 40,
                    depth: 40,
                    height: 40
                )
            }
        }
        .padding(8)
        .background(MirTheme.Colors.surfaceRaised)
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.medium).stroke(MirTheme.Colors.panelBorder, lineWidth: 1))
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
        .padding(10)
    }

    private func viewportButton(_ image: String, _ help: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: image)
                .font(.system(size: 11, weight: .medium))
                .frame(width: 26, height: 26)
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .background(MirTheme.Colors.surface)
                .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain)
        .help(help)
    }

    private var viewportReadout: some View {
        VStack {
            Spacer()
            HStack(spacing: 10) {
                Label("X\(format(cameraTheta))", systemImage: "arrow.left.and.right")
                Label("Y\(format(cameraPhi))", systemImage: "arrow.up.and.down")
                Label("D\(format(cameraDistance))", systemImage: "ruler")
                Spacer()
                Text(appState.ui.language == .russian
                    ? "\(isOrthographic ? "Ортографическая" : "Перспектива") · Колесо — масштаб · ПКМ — панорама"
                    : "\(isOrthographic ? "Orthographic" : "Perspective") · Wheel — zoom · RMB — pan")
                    .font(.system(size: 9))
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .font(.system(size: 9, weight: .medium, design: .monospaced))
            .foregroundStyle(MirTheme.Colors.textSecondary)
            .padding(.horizontal, 10)
            .padding(.vertical, 6)
            .background(MirTheme.Colors.surfaceRaised)
            .clipShape(Capsule())
            .overlay(Capsule().stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 1))
            .padding(10)
        }
        .allowsHitTesting(false)
    }

    private func format(_ value: Double) -> String { String(format: "%.1f", value) }
}