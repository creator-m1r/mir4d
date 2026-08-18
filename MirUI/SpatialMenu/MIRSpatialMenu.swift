import MirUIHandGesture
import SwiftUI
import AppKit
import Combine

/// The spatial menu coordinator.
///
/// Single flow for every input device:
/// ```text
/// Spatial Menu → MIRIntent → MIRIntentRouter → Command → MirEngine
/// ```
/// The coordinator owns presentation state only. CAD commands are executed
/// through the existing CADCommandRegistry / CADAppState / Event Bus layers;
/// MirEngine is never touched directly.
@MainActor
final class MIRSpatialMenuController: ObservableObject {
    static let shared = MIRSpatialMenuController()

    @Published var isPresented = false
    @Published var vector = CGVector.zero
    @Published private(set) var tree: [MIRSpatialMenuItem] = []
    @Published private(set) var context = MIRSpatialMenuSceneContext.idle
    @Published private(set) var state = MIRSpatialMenuState.initial
    @Published private(set) var voiceHint: String?
    @Published private(set) var language: String = "ru"

    private weak var appState: CADAppState?
    private weak var registry: CADCommandRegistry?
    private var previousState: MIRSpatialMenuState?
    private var observers: [NSObjectProtocol] = []
    private var voiceCancellable: AnyCancellable?
    private var lastHapticSignature: String?

    private init() {}

    // MARK: - Lifecycle

    func install(appState: CADAppState, registry: CADCommandRegistry) {
        self.appState = appState
        self.registry = registry
        language = appState.ui.language == .russian ? "ru" : "en"

        MIRSpatialMenuGesture.shared.start()
        MIRSpatialMenuVoiceAdapter.shared.connect(appState: appState)

        let center = NotificationCenter.default
        observers.append(center.addObserver(forName: .mir4DSpatialMenuBegan, object: nil, queue: .main) { [weak self] note in
            let via = note.userInfo?["via"] as? String ?? "system"
            Task { @MainActor [weak self] in self?.handleBegan(via: via) }
        })
        observers.append(center.addObserver(forName: .mir4DSpatialMenuMoved, object: nil, queue: .main) { [weak self] note in
            guard let dx = note.userInfo?["dx"] as? Double,
                  let dy = note.userInfo?["dy"] as? Double else { return }
            Task { @MainActor [weak self] in self?.handleMoved(dx: dx, dy: dy) }
        })
        observers.append(center.addObserver(forName: .mir4DSpatialMenuEnded, object: nil, queue: .main) { [weak self] note in
            let commit = (note.userInfo?["commit"] as? Bool) ?? false
            let dx = note.userInfo?["dx"] as? Double ?? 0
            let dy = note.userInfo?["dy"] as? Double ?? 0
            Task { @MainActor [weak self] in self?.handleEnded(commit: commit, dx: dx, dy: dy) }
        })

        voiceCancellable = MIRSpatialMenuVoiceAdapter.shared.$lastVoiceHint
            .receive(on: RunLoop.main)
            .sink { [weak self] hint in
                self?.voiceHint = hint
            }
    }

    func uninstall() {
        observers.forEach { NotificationCenter.default.removeObserver($0) }
        observers.removeAll()
        voiceCancellable = nil
        MIRSpatialMenuVoiceAdapter.shared.disconnect()
        MIRSpatialMenuGesture.shared.stop()
        isPresented = false
        vector = .zero
        state = .initial
        previousState = nil
    }

    // MARK: - Gesture handling

    private func handleBegan(via: String) {
        guard let appState else { return }
        context = MIRSpatialMenuContext.resolve(appState: appState)
        MIRSpatialMenuContextResolved.current = context
        tree = MIRSpatialMenuContext.tree(for: context)
        state = .initial
        previousState = nil
        vector = .zero
        isPresented = true
        lastHapticSignature = nil

        MIRIntentRouter.shared.publish(
            MIRIntent(source: .spatial, phase: .attention, action: "spatial.menu", confidence: 1.0)
        )
    }

    private func handleMoved(dx: Double, dy: Double) {
        guard isPresented else { return }
        vector = CGVector(dx: dx, dy: dy)
        let settings = MIRSpatialMenuSettingsStore.shared.settings
        let newState = MIRSpatialMenuSelection.state(
            vector: vector,
            tree: tree,
            settings: settings,
            previous: previousState
        )
        previousState = newState
        state = newState
        hapticIfNeeded(newState)

        publishPreviewIntent(for: newState)
    }

    private func handleEnded(commit: Bool, dx: Double, dy: Double) {
        guard isPresented else { return }
        vector = CGVector(dx: dx, dy: dy)
        state = MIRSpatialMenuSelection.state(
            vector: vector,
            tree: tree,
            settings: MIRSpatialMenuSettingsStore.shared.settings,
            previous: previousState
        )

        if commit, let tool = MIRSpatialMenuSelection.resolvedTool(state, tree: tree) {
            MIRIntentRouter.shared.publish(
                MIRIntent(source: .spatial, phase: .confirmation, action: tool.command, confidence: 1.0)
            )
            execute(command: tool.command)
            MIRIntentRouter.shared.publish(
                MIRIntent(source: .spatial, phase: .execution, action: tool.command, confidence: 1.0)
            )
        } else {
            MIRIntentRouter.shared.publish(
                MIRIntent(source: .spatial, phase: .cancel, action: "spatial.menu.cancel", confidence: 1.0)
            )
        }

        withAnimation(MIRSpatialMenuAnimation.disappear) {
            isPresented = false
            vector = .zero
            state = .initial
        }
        previousState = nil
    }

    // MARK: - Intents during the gesture

    private func publishPreviewIntent(for newState: MIRSpatialMenuState) {
        let settings = MIRSpatialMenuSettingsStore.shared.settings
        if let tool = MIRSpatialMenuSelection.resolvedTool(newState, tree: tree) {
            MIRIntentRouter.shared.publish(
                MIRIntent(source: .spatial, phase: .preview, action: tool.command, directionRadians: newState.angleRadians, confidence: 0.92)
            )
            return
        }
        if let index = newState.intentIndex, tree.indices.contains(index) {
            MIRIntentRouter.shared.publish(
                MIRIntent(source: .spatial, phase: .selection, action: tree[index].command, directionRadians: newState.angleRadians, confidence: 0.80)
            )
        }
    }

    // MARK: - Haptics

    private func hapticIfNeeded(_ newState: MIRSpatialMenuState) {
        guard MIRSpatialMenuSettingsStore.shared.settings.hapticEnabled else { return }
        let signature = "\(newState.level.rawValue):\(newState.intentIndex ?? -1):\(newState.categoryIndex ?? -1):\(newState.toolIndex ?? -1)"
        guard signature != lastHapticSignature else { return }
        lastHapticSignature = signature
        NSHapticFeedbackManager.defaultPerformer.perform(.alignment, performanceTime: .now)
    }

    // MARK: - Command execution

    /// Executes through the existing command layer only. Commands that are not
    /// registered fall back to the documented Event Bus / CADAppState channels.
    private func execute(command: String) {
        guard let appState else { return }
        let context = appState.activeContext

        if let registry, registry.execute(id: command, context: context) {
            return
        }

        switch command {
        case "create.sketch":
            appState.selectWorkbench(.sketch)
        case "history.undo", "edit.undo":
            MirEventBus.shared.publish(.undoRequested)
        case "history.redo", "edit.redo":
            MirEventBus.shared.publish(.redoRequested)
        case "sketch.line":
            activateSketchTool("line", appState: appState)
        case "sketch.rectangle":
            activateSketchTool("rectangle", appState: appState)
        case "sketch.circle":
            activateSketchTool("circle", appState: appState)
        case "sketch.arc":
            activateSketchTool("arc", appState: appState)
        case "sketch.constraint":
            activateSketchTool("constraint", appState: appState)
        case "sketch.dimension":
            activateSketchTool("dimension", appState: appState)
        case "object.delete":
            appState.clearSelection()
            MirEventBus.shared.publish(.commandRequested(command))
        default:
            MirEventBus.shared.publish(.commandRequested(command))
        }
    }

    private func activateSketchTool(_ tool: String, appState: CADAppState) {
        appState.selectWorkbench(.sketch)
        appState.selectedTool = tool
    }
}

// MARK: - Overlay view

/// Full-screen presentation of the spatial fan over the blurred scene.
struct MIRSpatialMenuView: View {
    @ObservedObject var controller: MIRSpatialMenuController
    @ObservedObject var settingsStore: MIRSpatialMenuSettingsStore

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                MIRSpatialMenuBlur(intensity: blurIntensity)

                MIRSpatialMenuRenderer(
                    tree: controller.tree,
                    state: controller.state,
                    settings: settingsStore.settings,
                    context: controller.context,
                    center: MIRSpatialMenuAnchor.center(in: proxy.size),
                    voiceHint: controller.voiceHint,
                    language: controller.language
                )
            }
        }
        .transition(.opacity.combined(with: .scale(scale: 0.96)))
    }

    private var blurIntensity: Double {
        let distance = controller.state.distance
        return min(max(distance / 240.0, 0.25), 1)
    }
}

// MARK: - Installer

/// Attaches the spatial menu to a workspace. While active it replaces the
/// legacy radial trigger path so the two menus never open at the same time.
struct MIRSpatialMenuInstaller: ViewModifier {
    let appState: CADAppState
    let registry: CADCommandRegistry

    func body(content: Content) -> some View {
        content
            .onAppear { MIRSpatialMenuController.shared.install(appState: appState, registry: registry) }
            .onDisappear { MIRSpatialMenuController.shared.uninstall() }
            .overlay {
                if MIRSpatialMenuController.shared.isPresented {
                    MIRSpatialMenuView(
                        controller: MIRSpatialMenuController.shared,
                        settingsStore: MIRSpatialMenuSettingsStore.shared
                    )
                    .zIndex(20)
                }
            }
    }
}

extension View {
    func mir4DSpatialMenu(appState: CADAppState, registry: CADCommandRegistry) -> some View {
        modifier(MIRSpatialMenuInstaller(appState: appState, registry: registry))
    }
}