import SwiftUI
import AppKit
import UniformTypeIdentifiers

private let mir4DCreativeImportTypes: [UTType] = [
    UTType(filenameExtension: "stl", conformingTo: .data), UTType(filenameExtension: "obj", conformingTo: .data),
    UTType(filenameExtension: "ply", conformingTo: .data), UTType(filenameExtension: "gltf", conformingTo: .data),
    UTType(filenameExtension: "glb", conformingTo: .data), UTType(filenameExtension: "fbx", conformingTo: .data),
    UTType(filenameExtension: "step", conformingTo: .data), UTType(filenameExtension: "stp", conformingTo: .data)
].compactMap { $0 }

/// Immersive MIR 4D workspace.
/// The scene is the product. Navigation is a quiet bottom dock; tools appear only on demand.
/// There are no permanent side/top tool panels and no dedicated conversational window.
struct MIR4DCreativeWorkspaceView: View {
    @ObservedObject var appState: CADAppState
    @StateObject private var registry = CADCommandRegistry()
    @StateObject private var productionStore = ProductionWorldStore()
    @State private var radialSettingsPresented = false
    @State private var showImporter = false
    @State private var cameraTheta = 0.0
    @State private var cameraPhi = 0.0
    @State private var cameraDistance = 1.0
    @State private var isOrthographic = false
    @State private var showEmptyState = true
    @State private var viewportSize: CGSize = .zero
    @State private var radialCenter: CGPoint = .zero
    @State private var radialVector: CGVector = .zero
    @State private var radialMenuPresented = false
    @State private var voiceActive = false

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        ZStack {
            viewport
            sceneOverlays
            navigationSphere
            emptyState
            bottomDock
            if radialMenuPresented { radialMenuOverlay }
        }
        .background(Color.black)
        .ignoresSafeArea()
        .sheet(isPresented: $radialSettingsPresented) { RadialMenuSettingsView(store: RadialMenuSettingsStore.shared) }
        .fileImporter(isPresented: $showImporter, allowedContentTypes: mir4DCreativeImportTypes, allowsMultipleSelection: false) { result in
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
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
        }
    }

    private var viewport: some View {
        GeometryReader { proxy in
            CreativeViewportRepresentable(appState: appState,
                onSelectionChanged: { objectID in appState.setSelection(ids: objectID > 0 ? ["\(objectID)"] : [], kind: objectID > 0 ? .body : .none) },
                onIOError: { message in appState.showNotification(message, type: .error) },
                onCameraOrientationChanged: { theta, phi, distance in
                    DispatchQueue.main.async { cameraTheta = theta; cameraPhi = phi; cameraDistance = distance; showEmptyState = false }
                })
                .onAppear { viewportSize = proxy.size }
                .onChange(of: proxy.size) { _, size in viewportSize = size }
        }
    }

    private var sceneOverlays: some View {
        ZStack {
            FourDSceneOverlayView(appState: appState).frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading).allowsHitTesting(false)
            if appState.workbench == .fourD {
                DigitalWorldHUD(appState: appState, store: productionStore)
                ProductionWorldView(appState: appState, store: productionStore).frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .bottom)
            }
            if appState.workbench == .sketch {
                SketchViewportView().environmentObject(appState).frame(maxWidth: .infinity, maxHeight: .infinity).allowsHitTesting(true)
                if appState.sketchPlane == nil { SketchPlaneChooser { appState.sketchPlane = $0 } }
            }
        }
    }

    private var navigationSphere: some View {
        NavigationSphereView(theta: cameraTheta, phi: cameraPhi, distance: cameraDistance, isOrthographic: isOrthographic)
            .opacity(0.38).scaleEffect(0.52).fixedSize()
            .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
            .padding(.top, 18).padding(.trailing, 18).allowsHitTesting(true)
    }

    @ViewBuilder private var emptyState: some View {
        if showEmptyState, appState.treeData.first?.children.isEmpty ?? true {
            VStack(spacing: 10) {
                Image(systemName: "sparkles.rectangle.stack").font(.system(size: 38, weight: .light)).foregroundStyle(.white.opacity(0.32))
                Text(russian ? "Представьте. Создайте. Изобретите." : "Imagine. Create. Invent.").font(.system(size: 21, weight: .medium)).foregroundStyle(.white.opacity(0.72))
                Text(russian ? "Сцена — ваше пространство для идеи." : "The scene is your space for ideas.").font(.system(size: 11)).foregroundStyle(.white.opacity(0.36))
                HStack(spacing: 8) {
                    Button { createDefaultBox() } label: { Label(russian ? "Начать" : "Start", systemImage: "plus") }.buttonStyle(.borderedProminent).controlSize(.small).tint(MirTheme.Colors.accent)
                    Button { showImporter = true } label: { Label(russian ? "Открыть модель" : "Open model", systemImage: "arrow.down.circle") }.buttonStyle(.bordered).controlSize(.small)
                }.padding(.top, 4)
            }
            .padding(26).background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 18))
            .overlay(RoundedRectangle(cornerRadius: 18).stroke(.white.opacity(0.10), lineWidth: 1))
            .frame(maxWidth: .infinity, maxHeight: .infinity).allowsHitTesting(true)
        }
    }

    private var bottomDock: some View {
        VStack(spacing: 0) {
            HStack(spacing: 4) {
                dockItem("cube.transparent", russian ? "Модель" : "Model", active: appState.workbench == .model) { appState.selectWorkbench(.model) }
                dockItem("pencil.and.ruler", russian ? "Эскиз" : "Sketch", active: appState.workbench == .sketch) { appState.selectWorkbench(.sketch) }
                dockItem("square.stack.3d.up", russian ? "Сборка" : "Assembly", active: appState.workbench == .assembly) { appState.selectWorkbench(.assembly) }
                dockItem("clock.arrow.circlepath", "4D", active: appState.workbench == .fourD) { appState.selectWorkbench(.fourD) }
                dockItem("waveform.path.ecg", russian ? "Расчёт" : "Simulation", active: appState.workbench == .simulation) { appState.selectWorkbench(.simulation) }
                dockDivider
                voiceDockItem
                dockDivider
                dockItem("folder", russian ? "Проект" : "Project", active: false) { NotificationCenter.default.post(name: .mir4DProjectClosed, object: nil) }
            }
            .padding(.horizontal, 12)
            .padding(.vertical, 9)
            .background(.ultraThinMaterial, in: Capsule())
            .overlay(Capsule().stroke(.white.opacity(0.14), lineWidth: 1))
            .shadow(color: .black.opacity(0.48), radius: 24, y: 11)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .bottom)
        .padding(.bottom, 14)
        .allowsHitTesting(true)
    }

    private var voiceDockItem: some View {
        Button {
            voiceActive.toggle()
            NotificationCenter.default.post(name: .mir4DVoiceInteractionRequested, object: nil, userInfo: ["active": voiceActive])
        } label: {
            ZStack {
                Circle()
                    .fill(voiceActive ? MirTheme.Colors.accent.opacity(0.30) : Color.white.opacity(0.07))
                    .frame(width: 48, height: 48)
                Circle()
                    .stroke(voiceActive ? MirTheme.Colors.accent : Color.white.opacity(0.16), lineWidth: voiceActive ? 1.5 : 1)
                    .frame(width: 48, height: 48)
                Image(systemName: voiceActive ? "waveform" : "mic")
                    .font(.system(size: 17, weight: .semibold))
                    .foregroundStyle(voiceActive ? MirTheme.Colors.accentBright : .white.opacity(0.72))
            }
        }
        .buttonStyle(.plain)
        .frame(width: 58, height: 48)
        .help(russian ? "Голосовое взаимодействие" : "Voice interaction")
        .animation(.easeOut(duration: 0.18), value: voiceActive)
    }

    private var dockDivider: some View { Rectangle().fill(.white.opacity(0.10)).frame(width: 1, height: 28).padding(.horizontal, 4) }

    private func dockItem(_ icon: String, _ title: String, active: Bool, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            VStack(spacing: 3) {
                Image(systemName: icon).font(.system(size: 14, weight: .semibold))
                Text(title).font(.system(size: 8, weight: .medium))
            }
            .foregroundStyle(active ? .white : .white.opacity(0.55))
            .frame(width: 62, height: 42)
            .background(active ? MirTheme.Colors.selection.opacity(0.72) : Color.clear, in: RoundedRectangle(cornerRadius: 10))
        }
        .buttonStyle(.plain)
        .help(title)
    }

    private var radialMenuOverlay: some View {
        RadialMenuView(store: RadialMenuSettingsStore.shared, center: radialCenter, vector: radialVector, onToolActivated: { _ in radialMenuPresented = false }, onSettings: { radialSettingsPresented = true })
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(Color.black.opacity(0.10).allowsHitTesting(false))
    }

    private func createDefaultBox() {
        _ = MIR4DModelCommands.shared.createBox(appState: appState, width: 40, depth: 40, height: 40)
    }
}

private struct CreativeViewportRepresentable: NSViewRepresentable {
    var appState: CADAppState
    var onSelectionChanged: (UInt64) -> Void
    var onIOError: (String) -> Void
    var onCameraOrientationChanged: (Double, Double, Double) -> Void
    func makeNSView(context: Context) -> MirGLCustomView { let view = MirGLCustomView(); apply(to: view); return view }
    func updateNSView(_ nsView: MirGLCustomView, context: Context) { apply(to: nsView) }
    private func apply(to view: MirGLCustomView) { view.appState = appState; view.onSelectionChanged = onSelectionChanged; view.onIOError = onIOError; view.onCameraOrientationChanged = onCameraOrientationChanged; view.syncWorkPlanesIfNeeded() }
}
