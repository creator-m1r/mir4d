import SwiftUI
import AppKit
import UniformTypeIdentifiers

private let mir4DImportTypes: [UTType] = [
    UTType(filenameExtension: "stl", conformingTo: .data), UTType(filenameExtension: "obj", conformingTo: .data), UTType(filenameExtension: "ply", conformingTo: .data), UTType(filenameExtension: "gltf", conformingTo: .data), UTType(filenameExtension: "glb", conformingTo: .data), UTType(filenameExtension: "fbx", conformingTo: .data), UTType(filenameExtension: "step", conformingTo: .data), UTType(filenameExtension: "stp", conformingTo: .data)
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
    @State private var isOrthographic = false
    @State private var showEmptyState = true
    @State private var viewportSize: CGSize = .zero
    @State private var radialCenter: CGPoint = .zero
    @State private var radialVector: CGVector = .zero
    @State private var radialMenuPresented = false
    @State private var resizeBaseline: [ResizeEdge: Double] = [:]

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                mainLayout
                if radialMenuPresented { radialMenuOverlay }
            }
            .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuBegan)) { _ in
                radialCenter = CGPoint(x: proxy.size.width * 0.5, y: proxy.size.height * 0.5)
                radialVector = .zero
                radialMenuPresented = true
            }
            .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuMoved)) { note in
                guard radialMenuPresented,
                      let dx = note.userInfo?["dx"] as? Double,
                      let dy = note.userInfo?["dy"] as? Double else { return }
                radialVector = CGVector(dx: dx, dy: dy)
            }
            .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuEnded)) { note in
                if let command = note.userInfo?["command"] as? String {
                    executeRadialCommand(command)
                }
                radialMenuPresented = false
                radialVector = .zero
            }
        }
        .background(MirTheme.Colors.background)
        .mir4DRadialKeyboardTrigger()
        .sheet(isPresented: $commandPalettePresented) { CommandPaletteView(appState: appState, registry: registry) }
        .sheet(isPresented: $radialSettingsPresented) { RadialMenuSettingsView(store: RadialMenuSettingsStore.shared) }
        .fileImporter(isPresented: $showImporter, allowedContentTypes: mir4DImportTypes, allowsMultipleSelection: false) { result in
            guard case .success(let urls) = result, let url = urls.first else { return }
            NotificationCenter.default.post(name: .mir4DImportMesh, object: url.path)
        }
        .onChange(of: appState.panelState) { _, _ in FloatingPanelManager.shared.sync(appState: appState) }
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
            FloatingPanelManager.shared.sync(appState: appState)
        }
        .onDisappear { FloatingPanelManager.shared.closeAll() }
    }

    private var mainLayout: some View {
        ZStack(alignment: .topTrailing) {
            centerColumn

            MIR4DViewportActionDock(
                onCommandPalette: { commandPalettePresented = true },
                onSettings: { radialSettingsPresented = true },
                onPanels: { workspace.isMinimalMode.toggle() }
            )
            .padding(.top, 14)
            .padding(.trailing, 14)
        }
    }

    private var centerColumn: some View {
        MIR4DInteractionSurface(
            onRadialCommit: { vector in
                radialCenter = CGPoint(x: viewportSize.width * 0.5, y: viewportSize.height * 0.5)
                radialVector = vector
                radialMenuPresented = true
            },
            onCameraOrbitDelta: applyCameraIntent(deltaTheta:deltaPhi:deltaDistance:)
        ) {
            ZStack {
                viewport
                viewportOverlay
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
                onSelectionChanged: { objectID, kind, elementId in
                    let mapped: CADSelectionKind = {
                        switch kind {
                            case 1: return .vertex
                            case 2: return .edge
                            case 3: return .face
                            case 4: return .body
                            default: return .none
                        }
                    }()
                    let finalKind: CADSelectionKind = objectID > 0 ? mapped : .none
                    appState.setSelection(
                        ids: objectID > 0 ? ["\(objectID)"] : [],
                        kind: finalKind,
                        elementId: objectID > 0 ? elementId : 0
                    )
                },
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
                ProductionWorldView(appState: appState, store: productionStore)
                    .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .bottom)
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
        .opacity(0.65)
        .scaleEffect(0.56)
        .fixedSize()
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
        .padding(.top, 14)
        .padding(.trailing, 14)
    }

    @ViewBuilder private var emptyStateHint: some View {
        if showEmptyState, appState.treeData.first?.children.isEmpty ?? true {
            VStack(spacing: 16) {
                Image(systemName: "cube.transparent")
                    .font(.system(size: 42, weight: .light))
                    .foregroundStyle(MirTheme.Colors.textTertiary)
                Text(appState.ui.language == .russian ? "Рабочая область свободна" : "Workspace is empty")
                    .font(MirTheme.Typography.title)
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                Text(appState.ui.language == .russian ? "Создайте первый инженерный объект или импортируйте модель." : "Create your first engineering object or import a model.")
                    .font(MirTheme.Typography.caption)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
                HStack(spacing: 10) {
                    Button { createDefaultBox() } label: {
                        Label(appState.ui.language == .russian ? "Новое тело" : "New Body", systemImage: "cube.transparent")
                    }
                    .buttonStyle(.borderedProminent)
                    .controlSize(.small)
                    .tint(MirTheme.Colors.accent)

                    Button { showImporter = true } label: {
                        Label(appState.ui.language == .russian ? "Импорт" : "Import", systemImage: "square.and.arrow.down")
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
        MIR4DRadialSelectionOverlay(
            store: RadialMenuSettingsStore.shared,
            center: radialCenter,
            vector: radialVector,
            onToolActivated: { tool in
                executeRadialCommand(tool.command)
                radialMenuPresented = false
                radialVector = .zero
            },
            onSettings: { radialSettingsPresented = true }
        )
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    private func executeRadialCommand(_ command: String) {
        let normalized = command.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !normalized.isEmpty else { return }
        guard registry.execute(id: normalized, context: appState.activeContext) else {
            appState.showNotification("Команда недоступна в текущем контексте: \(normalized)", type: .warning)
            return
        }
    }

    private func applyCameraIntent(deltaTheta: Double, deltaPhi: Double, deltaDistance: Double) {
        let nextTheta = cameraTheta + deltaTheta
        let nextPhi = min(max(cameraPhi + deltaPhi, -1.52), 1.52)
        let nextDistance = min(max(cameraDistance * (deltaDistance == 0 ? 1 : deltaDistance), 0.05), 10_000)

        cameraTheta = nextTheta
        cameraPhi = nextPhi
        cameraDistance = nextDistance
        showEmptyState = false
        Mir4DSetCameraOrbit(theta: nextTheta, phi: nextPhi, distance: nextDistance)
    }

    private func createDefaultBox() {
        _ = MIR4DModelCommands.shared.createBox(appState: appState, width: 100, depth: 60, height: 40)
        showEmptyState = false
    }

    private enum ResizeEdge: Hashable { case left, right, bottom }
}

// MARK: - Viewport bridge

/// SwiftUI wrapper around the MirGLCustomView OpenGL viewport.
/// Camera and selection are forwarded through the CADAppState callbacks.
struct ViewportRepresentable: NSViewRepresentable {
    var appState: CADAppState
    var onSelectionChanged: (UInt64, Int32, UInt64) -> Void
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
        view.appState = appState
        view.onSelectionChanged = onSelectionChanged
        view.onIOError = onIOError
        view.onCameraOrientationChanged = onCameraOrientationChanged
        view.syncWorkPlanesIfNeeded()
    }
}
