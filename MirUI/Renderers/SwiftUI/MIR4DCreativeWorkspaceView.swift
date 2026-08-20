import SwiftUI
import AppKit
import UniformTypeIdentifiers
import MirUIHandGesture

private let mir4DCreativeImportTypes: [UTType] = [
    UTType(filenameExtension: "stl", conformingTo: .data), UTType(filenameExtension: "obj", conformingTo: .data),
    UTType(filenameExtension: "ply", conformingTo: .data), UTType(filenameExtension: "gltf", conformingTo: .data),
    UTType(filenameExtension: "glb", conformingTo: .data), UTType(filenameExtension: "fbx", conformingTo: .data),
    UTType(filenameExtension: "step", conformingTo: .data), UTType(filenameExtension: "stp", conformingTo: .data)
].compactMap { $0 }

/// Immersive MIR 4D workspace.
/// The scene is the product. Navigation is deliberately reduced to a quiet bottom dock;
/// tools appear in the center only while held. Voice is an ambient system capability.
struct MIR4DCreativeWorkspaceView: View {
    @ObservedObject var appState: CADAppState
    @StateObject private var registry = CADCommandRegistry()
    @StateObject private var productionStore = ProductionWorldStore()
    @StateObject private var voiceAssistant = MIR4DVoiceAssistant()
    @State private var radialSettingsPresented = false
    @State private var showImporter = false
    @State private var cameraTheta = 0.0
    @State private var cameraPhi = 0.0
    @State private var cameraDistance = 1.0
    @State private var isOrthographic = false
    @State private var showEmptyState = true
    @State private var radialVector: CGVector = .zero
    @State private var radialMenuPresented = false
    @State private var skeletonMenuPresented = false
    @State private var selectedSkeletonMode: MIRHandSkeletonVisMode = .off

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
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuBegan)) { _ in
            withAnimation(.easeOut(duration: 0.18)) {
                radialVector = .zero
                radialMenuPresented = true
            }
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuMoved)) { note in
            guard radialMenuPresented,
                  let dx = note.userInfo?["dx"] as? Double,
                  let dy = note.userInfo?["dy"] as? Double else { return }
            radialVector = CGVector(dx: dx, dy: dy)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuEnded)) { note in
            let commit = (note.userInfo?["commit"] as? Bool) ?? false
            let finalVector = CGVector(
                dx: (note.userInfo?["dx"] as? Double) ?? radialVector.dx,
                dy: (note.userInfo?["dy"] as? Double) ?? radialVector.dy
            )
            if commit { commitRadialSelection(vector: finalVector) }
            withAnimation(.easeIn(duration: 0.16)) {
                radialMenuPresented = false
                radialVector = .zero
            }
        }
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
            if MIR4DProjectPermissions.shared.microphoneEnabled {
                voiceAssistant.start(appState: appState)
            }
            MIR4DRadialInteractionCoordinator.shared.start()
            // Hand tracking is started here — not during the launch transition —
            // so the camera capture subsystem comes up only once the workspace
            // is stable, and only when the user enabled it. `startCamera()` is
            // idempotent: it no-ops if already running.
            if MIR4DProjectPermissions.shared.cameraEnabled {
                DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                    MIRHandGestureModule.shared.startCamera()
                }
            }
        }
        .mir4DSpatialMenu(appState: appState, registry: registry)
        .onDisappear {
            voiceAssistant.stop()
            MIR4DRadialInteractionCoordinator.shared.stop()
        }
    }

    private var viewport: some View {
        GeometryReader { _ in
            CreativeViewportRepresentable(appState: appState,
                onSelectionChanged: { objectID in appState.setSelection(ids: objectID > 0 ? ["\(objectID)"] : [], kind: objectID > 0 ? .body : .none) },
                onIOError: { message in appState.showNotification(message, type: .error) },
                onCameraOrientationChanged: { theta, phi, distance in
                    DispatchQueue.main.async { cameraTheta = theta; cameraPhi = phi; cameraDistance = distance; showEmptyState = false }
                })
        }
    }

    private var sceneOverlays: some View {
        ZStack {
            FourDSceneOverlayView(appState: appState)
                .padding(.top, (35 + 50) * 2.834645669)
                .padding(.leading, 6 * 2.834645669)
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
                .allowsHitTesting(false)
            MIR4DBrushOverlay()
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .allowsHitTesting(false)
            MIR4DSculptSettingsHUD()
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

    /// The sphere is a quiet spatial instrument in the upper-left corner.
    private var navigationSphere: some View {
        NavigationSphereView(theta: cameraTheta, phi: cameraPhi, distance: cameraDistance, isOrthographic: isOrthographic)
            .opacity(0.16)
            .scaleEffect(0.44)
            .fixedSize()
            .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
            .padding(.top, 12)
            .padding(.leading, 12)
            .allowsHitTesting(true)
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

    /// The dock is navigation between worlds, not a command bar.
    private var bottomDock: some View {
        HStack(spacing: 3) {
            dockItem("cube.transparent", russian ? "Модель" : "Model", active: appState.workbench == .model) { appState.selectWorkbench(.model) }
            dockItem("pencil.and.ruler", russian ? "Эскиз" : "Sketch", active: appState.workbench == .sketch) { appState.selectWorkbench(.sketch) }
            dockItem("clock.arrow.circlepath", "4D", active: appState.workbench == .fourD) { appState.selectWorkbench(.fourD) }
            dockItem("folder", russian ? "Проект" : "Project", active: false) { NotificationCenter.default.post(name: .mir4DProjectClosed, object: nil) }

            Divider()
                .frame(height: 26)
                .overlay(Color.white.opacity(0.11))

            dockItem("hand.raised", russian ? "Скелет" : "Skeleton",
                     active: selectedSkeletonMode != .off) { skeletonMenuPresented = true }
        }
        .padding(.horizontal, 9)
        .padding(.vertical, 7)
        .background(.ultraThinMaterial, in: Capsule())
        .overlay(Capsule().stroke(.white.opacity(0.11), lineWidth: 1))
        .shadow(color: .black.opacity(0.42), radius: 20, y: 9)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .bottom)
        .padding(.bottom, 12)
        .allowsHitTesting(true)
        .popover(isPresented: $skeletonMenuPresented) { skeletonSettingsPopover }
    }

    // MARK: - Hand skeleton visualization settings

    private var skeletonSettingsPopover: some View {
        VStack(alignment: .leading, spacing: 14) {
            Text(russian ? "Визуализация скелета кисти" : "Hand skeleton visualization")
                .font(.headline)

            Button {
                Task { @MainActor in
                    let granted = await MIR4DSystemCaptureAuthorizationViewModel().requestCamera()
                    if granted {
                        MIR4DProjectPermissions.shared.cameraEnabled = true
                        MIRHandGestureModule.shared.startCamera()
                        appState.showNotification(
                            russian ? "Камера включена" : "Camera enabled",
                            type: .success)
                    } else {
                        appState.showNotification(
                            russian ? "Нет доступа к камере. Разрешите в Системных настройках." : "No camera access. Enable it in System Settings.",
                            type: .warning)
                    }
                }
            } label: {
                Label(russian ? "Включить камеру" : "Enable camera",
                      systemImage: "camera")
            }
            .controlSize(.small)

            Picker(russian ? "Режим" : "Mode", selection: skeletonMode) {
                Text("Выкл").tag(MIRHandSkeletonVisMode.off)
                Text("Точки").tag(MIRHandSkeletonVisMode.jointsOnly)
                Text("Кости").tag(MIRHandSkeletonVisMode.bones)
                Text("Кости + луч").tag(MIRHandSkeletonVisMode.bonesAndRays)
            }
            .pickerStyle(.segmented)

            Toggle(russian ? "Глубина (depth test)" : "Depth test", isOn: skeletonDepthTest)

            VStack(alignment: .leading, spacing: 4) {
                Text(russian ? "Размер сустава" : "Joint size")
                    .font(.caption)
                Slider(value: skeletonJointSize, in: 1...16, step: 1)
            }

            VStack(alignment: .leading, spacing: 4) {
                Text(russian ? "Прозрачность" : "Alpha")
                    .font(.caption)
                Slider(value: skeletonAlpha, in: 0.1...1)
            }

            Text(russian ? "Требуется запущенная камера (режим рук)" : "Requires a running camera (hand mode)")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .padding(16)
        .frame(width: 320)
    }

    private var skeletonMode: Binding<MIRHandSkeletonVisMode> {
        Binding(
            get: { MIRHandGestureModule.shared.configuration.skeletonVisualizationMode },
            set: { newValue in
                MIRHandGestureModule.shared.setSkeletonVisualizationMode(newValue)
                selectedSkeletonMode = newValue
            }
        )
    }

    private var skeletonDepthTest: Binding<Bool> {
        Binding(
            get: { MIRHandGestureModule.shared.configuration.skeletonDepthTest },
            set: { MIRHandGestureModule.shared.configuration.skeletonDepthTest = $0 }
        )
    }

    private var skeletonJointSize: Binding<Double> {
        Binding(
            get: { MIRHandGestureModule.shared.configuration.skeletonJointSize },
            set: { MIRHandGestureModule.shared.configuration.skeletonJointSize = $0 }
        )
    }

    private var skeletonAlpha: Binding<Double> {
        Binding(
            get: { MIRHandGestureModule.shared.configuration.skeletonAlpha },
            set: { MIRHandGestureModule.shared.configuration.skeletonAlpha = $0 }
        )
    }

    private func dockItem(_ icon: String, _ title: String, active: Bool, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            VStack(spacing: 2) {
                Image(systemName: icon).font(.system(size: 14, weight: .semibold))
                Text(title).font(.system(size: 8, weight: .medium))
            }
            .foregroundStyle(active ? .white : .white.opacity(0.52))
            .frame(width: 66, height: 40)
            .background(active ? MirTheme.Colors.selection.opacity(0.68) : Color.clear, in: RoundedRectangle(cornerRadius: 10))
        }
        .buttonStyle(.plain)
        .help(title)
    }

    private var radialMenuOverlay: some View {
        MIR4DRadialCenterOverlay(
            store: RadialMenuSettingsStore.shared,
            vector: radialVector,
            onToolActivated: { _ in radialMenuPresented = false },
            onSettings: { radialSettingsPresented = true }
        )
    }

    private func commitRadialSelection(vector: CGVector) {
        let settings = RadialMenuSettingsStore.shared.settings
        guard let panelIndex = RadialMenuGeometry.panelIndex(for: vector.dx, dy: vector.dy, settings: settings) else { return }
        let panels = RadialMenuGeometry.enabledPanels(settings)
        guard panels.indices.contains(panelIndex) else { return }
        let panel = panels[panelIndex]

        if let toolIndex = RadialMenuGeometry.toolIndex(for: vector.dx, dy: vector.dy, panel: panel, settings: settings), panel.tools.indices.contains(toolIndex) {
            MirEventBus.shared.publish(.commandRequested(panel.tools[toolIndex].command))
            return
        }

        switch panel.title {
        case "Модель": appState.selectWorkbench(.model)
        case "Сборка": appState.selectWorkbench(.assembly)
        case "Симуляция": appState.selectWorkbench(.simulation)
        case "4D": appState.selectWorkbench(.fourD)
        case "Чертёж": appState.selectWorkbench(.drawing)
        default: break
        }
    }

    private func createDefaultBox() { _ = MIR4DModelCommands.shared.createBox(appState: appState, width: 40, depth: 40, height: 40) }
}

private struct CreativeViewportRepresentable: NSViewRepresentable {
    var appState: CADAppState
    var onSelectionChanged: (UInt64) -> Void
    var onIOError: (String) -> Void
    var onCameraOrientationChanged: (Double, Double, Double) -> Void
    func makeNSView(context: Context) -> MirGLCustomView { let view = MirGLCustomView(); view.appState = appState; view.onSelectionChanged = onSelectionChanged; view.onIOError = onIOError; view.onCameraOrientationChanged = onCameraOrientationChanged; return view }
    func updateNSView(_ nsView: MirGLCustomView, context: Context) { nsView.appState = appState; nsView.onSelectionChanged = onSelectionChanged; nsView.onIOError = onIOError; nsView.onCameraOrientationChanged = onCameraOrientationChanged; nsView.syncWorkPlanesIfNeeded() }
}
