// MirUI/Renderers/SwiftUI/CADAppState.swift

import SwiftUI
import Combine
import AppKit

// MARK: - MIR 4D Time Coordinator

/// Coordinates presentation time for the 4D interface.
///
/// Engineering truth remains in MirEngine.
/// This object only represents UI/runtime time state.
@MainActor
final class TimeCoordinator: ObservableObject {

    // MARK: Published state

    @Published private(set) var currentTime: Double = 0.0
    @Published private(set) var isPlaying: Bool = false

    @Published private(set) var scenarioID: String = "scenario-main"
    @Published private(set) var branchID: String = "branch-main"

    // MARK: Time configuration

    let startTime: Double = 0.0
    let endTime: Double = 10.0

    let tickInterval: TimeInterval = 1.0 / 60.0

    // MARK: Runtime

    private var timer: Timer?

    // MARK: Initialization

    init() {}

    // MARK: Seeking

    func seek(_ time: Double) {

        currentTime = min(
            max(
                time,
                startTime
            ),
            endTime
        )

        publishTimeChanged()
    }

    func step(by delta: Double) {
        seek(currentTime + delta)
    }

    // MARK: Playback

    func play() {

        guard !isPlaying else {
            return
        }

        if currentTime >= endTime {
            currentTime = startTime
        }

        isPlaying = true

        publishTimeChanged()

        // Always invalidate an existing timer first.
        timer?.invalidate()
        timer = nil

        timer = Timer.scheduledTimer(
            withTimeInterval: tickInterval,
            repeats: true
        ) { [weak self] _ in

            // Do not move Timer or NSEvent through concurrency.
            // Only call back into the MainActor coordinator.
            Task { @MainActor [weak self] in

                guard let self else {
                    return
                }

                let next =
                    self.currentTime
                    + self.tickInterval

                if next >= self.endTime {

                    self.seek(
                        self.endTime
                    )

                    self.pause()

                } else {

                    self.seek(
                        next
                    )
                }
            }
        }
    }

    func pause() {

        isPlaying = false

        timer?.invalidate()
        timer = nil

        publishTimeChanged()
    }

    func togglePlayback() {

        if isPlaying {
            pause()
        } else {
            play()
        }
    }

    // MARK: Time control

    func reset() {

        pause()

        currentTime =
            startTime

        publishTimeChanged()
    }

    func finish() {

        pause()

        currentTime =
            endTime

        publishTimeChanged()
    }

    // MARK: Scenario

    func createScenario() {

        scenarioID =
            "scenario-\(UUID().uuidString.prefix(8).lowercased())"

        branchID =
            "branch-main"

        publishTimeChanged()
    }

    func createBranch() {

        branchID =
            "branch-\(UUID().uuidString.prefix(8).lowercased())"

        publishTimeChanged()
    }

    // MARK: Snapshot

    var snapshot: CADTimeState {

        CADTimeState(
            current: currentTime,
            start: startTime,
            end: endTime,
            isPlaying: isPlaying,
            scenarioID: scenarioID,
            branchID: branchID
        )
    }

    // MARK: Event bus

    private func publishTimeChanged() {

        MirEventBus.shared.publish(
            .timeChanged(snapshot)
        )
    }
}

// MARK: - Notifications

enum NotificationType {
    case info
    case success
    case warning
    case error
}

struct CADNotification: Identifiable {

    let id = UUID()

    let message: String

    let type: NotificationType
}

// MARK: - CAD Application State

/// MIR 4D UI state.
///
/// This is a projection of engine state plus presentation state.
/// Engineering truth remains in MirEngine.
@MainActor
final class CADAppState: ObservableObject {

    // MARK: Workbench

    @Published var workbench: CADWorkbench = .model

    @Published var subMode: CADSubMode = .modelFeature

    @Published var selectedTool: String = "select"

    // MARK: Selection

    @Published var selection =
        CADSelectionState()

    // MARK: Simulation

    @Published var simulation =
        CADSimulationState()

    // MARK: Interaction

    @Published var interaction =
        CADInteractionState.forWorkbench(.model)

    // MARK: UI

    @Published var ui =
        CADUIState()

    @Published var panelState =
        CADPanelState.forWorkbench(.model)

    @Published var selectedTreeItem: String =
        "Система жизнеобеспечения"

    @Published var activePropTab: Int = 0

    // MARK: Notifications

    @Published var notifications:
        [CADNotification] = []

    // MARK: Document

    @Published var documentName:
        String = "Новый проект"

    @Published var documentDirty:
        Bool = false

    // MARK: Viewport

    @Published var gridVisible:
        Bool = true

    @Published var axesVisible:
        Bool = true

    @Published var sectionMode:
        Bool = false

    // MARK: Time

    let time =
        TimeCoordinator()

    // MARK: Combine

    private var cancellables =
        Set<AnyCancellable>()

    // MARK: Derived state

    var visiblePanels:
        Set<CADPanel> {

        panelState.visible
    }

    var selectionCount:
        Int {

        selection.count
    }

    var currentTime:
        Double {

        time.currentTime
    }

    var isPlaying:
        Bool {

        time.isPlaying
    }

    var timeState:
        CADTimeState {

        time.snapshot
    }

    // MARK: Active context

    var activeContext:
        CADActiveContext {

        CADActiveContext(
            workbench: workbench,
            subMode: subMode,
            selection: selection,
            time: timeState,
            interaction: interaction,
            simulation: simulation,
            experience: ui.experience,
            language: ui.language
        )
    }

    // MARK: Project tree

    let treeData: [TreeNodeData] = [

        TreeNodeData(
            name: "Проект",
            icon: "cube",
            children: [

                TreeNodeData(
                    name: "Космический шлем",
                    icon: "square.stack.3d.up",
                    status: .inProgress,
                    children: [

                        TreeNodeData(
                            name: "Корпус",
                            icon: "square",
                            status: .approved
                        ),

                        TreeNodeData(
                            name: "Визор",
                            icon: "square",
                            status: .approved
                        ),

                        TreeNodeData(
                            name: "Система жизнеобеспечения",
                            icon: "square",
                            status: .inProgress
                        )
                    ]
                ),

                TreeNodeData(
                    name: "Сборочные единицы",
                    icon: "rectangle.split.3x1",
                    status: .approved
                ),

                TreeNodeData(
                    name: "Электроника",
                    icon: "cpu",
                    status: .issue
                ),

                TreeNodeData(
                    name: "Конструкторская документация",
                    icon: "doc.text"
                )
            ]
        )
    ]

    // MARK: Initialization

    init() {

        time.objectWillChange
            .receive(on: RunLoop.main)
            .sink { [weak self] _ in

                self?.objectWillChange.send()
            }
            .store(
                in: &cancellables
            )
    }

    // MARK: Workbench

    func selectWorkbench(
        _ value: CADWorkbench
    ) {

        workbench =
            value

        subMode =
            value.defaultSubMode

        selectedTool =
            value.defaultTool

        panelState =
            .forWorkbench(value)

        interaction =
            .forWorkbench(value)

        activePropTab =
            0

        MirEventBus.shared.publish(
            .workbenchChanged(value)
        )

        MirEventBus.shared.publish(
            .subModeChanged(subMode)
        )

        showNotification(
            ui.language == .russian
                ? "Рабочая среда: \(value.titleRU)"
                : "Workbench: \(value.titleEN)",
            type: .success
        )
    }

    func switchSubMode(
        to value: CADSubMode
    ) {

        guard value.workbench == workbench else {
            return
        }

        subMode =
            value

        selectedTool =
            value.defaultTool

        MirEventBus.shared.publish(
            .subModeChanged(value)
        )
    }

    // MARK: Panels

    func togglePanel(
        _ panel: CADPanel
    ) {

        panelState.toggle(
            panel
        )
    }

    // MARK: UI

    func toggleLanguage() {

        ui.language =
            ui.language == .russian
                ? .english
                : .russian
    }

    func toggleExperience() {

        ui.experience =
            ui.experience == .expert
                ? .beginner
                : .expert
    }

    // MARK: Selection

    func setSelection(
        ids: [String],
        kind: CADSelectionKind
    ) {

        selection =
            CADSelectionState(
                ids: ids,
                primaryKind: kind
            )

        MirEventBus.shared.publish(
            .selectionChanged(selection)
        )
    }

    func clearSelection() {

        selection =
            CADSelectionState()

        MirEventBus.shared.publish(
            .selectionChanged(selection)
        )
    }

    // MARK: Viewport

    func toggleGrid() {
        gridVisible.toggle()
    }

    func toggleAxes() {
        axesVisible.toggle()
    }

    func toggleSection() {
        sectionMode.toggle()
    }

    // MARK: Document

    func newDocument() {

        documentName =
            "Новый проект"

        documentDirty =
            false

        selectedTreeItem =
            "Проект"

        clearSelection()

        simulation =
            CADSimulationState()

        time.reset()

        MirEventBus.shared.publish(
            .documentChanged
        )

        showNotification(
            ui.language == .russian
                ? "Создан новый проект"
                : "New project created",
            type: .success
        )
    }

    func markDirty() {

        documentDirty =
            true

        MirEventBus.shared.publish(
            .documentChanged
        )
    }

    // MARK: Import / Export

    func importModel(
        url: URL
    ) {

        documentDirty =
            true

        MirEventBus.shared.publish(
            .commandRequested(
                "document.import"
            )
        )

        showNotification(
            ui.language == .russian
                ? "Импорт: \(url.lastPathComponent)"
                : "Import: \(url.lastPathComponent)",
            type: .info
        )
    }

    func exportModel() {

        MirEventBus.shared.publish(
            .commandRequested(
                "document.export"
            )
        )

        showNotification(
            ui.language == .russian
                ? "Экспорт модели подготовлен"
                : "Model export prepared",
            type: .success
        )
    }

    func loadModel(
        url: URL
    ) {

        importModel(
            url: url
        )
    }

    // MARK: 4D Time

    func seek(
        _ seconds: Double
    ) {

        time.seek(
            seconds
        )
    }

    func togglePlayback() {

        time.togglePlayback()
    }

    func stepBackward() {

        time.step(
            by: -1.0
        )
    }

    func stepForward() {

        time.step(
            by: 1.0
        )
    }

    func resetTime() {

        time.reset()
    }

    func finishTime() {

        time.finish()
    }

    func createTimeScenario() {

        time.createScenario()

        MirEventBus.shared.publish(
            .commandRequested(
                "fourD.scenario"
            )
        )
    }

    func createTimeBranch() {

        time.createBranch()

        MirEventBus.shared.publish(
            .commandRequested(
                "fourD.branch"
            )
        )
    }

    // MARK: Simulation

    func setPhysics(
        _ physics: CADSimulationState.PhysicsType
    ) {

        simulation.physics =
            physics

        MirEventBus.shared.publish(
            .simulationChanged(simulation)
        )
    }

    func setSimulationPhase(
        _ phase: CADSimulationState.Phase
    ) {

        simulation.phase =
            phase

        MirEventBus.shared.publish(
            .simulationChanged(simulation)
        )
    }

    func updateSimulationProgress(
        _ progress: Double
    ) {

        simulation.progress =
            min(
                max(
                    progress,
                    0
                ),
                1
            )

        MirEventBus.shared.publish(
            .simulationChanged(simulation)
        )
    }

    func finishSimulation(
        resultSetID: String? = nil
    ) {

        simulation.isRunning =
            false

        simulation.progress =
            1

        simulation.resultSetID =
            resultSetID
            ?? "result-\(UUID().uuidString.prefix(8).lowercased())"

        simulation.phase =
            .results

        simulation.solverStatus =
            ui.language == .russian
                ? "Расчёт завершён"
                : "Solve completed"

        MirEventBus.shared.publish(
            .simulationChanged(simulation)
        )
    }

    // MARK: Notifications

    func showNotification(
        _ msg: String,
        type: NotificationType = .info
    ) {

        let notification =
            CADNotification(
                message: msg,
                type: type
            )

        notifications.append(
            notification
        )

        Task { @MainActor [weak self] in

            try? await Task.sleep(
                for: .seconds(3.5)
            )

            guard let self else {
                return
            }

            notifications.removeAll {
                $0.id == notification.id
            }
        }
    }
}

// MARK: - Project Tree Node

struct TreeNodeData: Identifiable {

    let id =
        UUID()

    let name:
        String

    let icon:
        String

    var status:
        Status = .none

    var children:
        [TreeNodeData] = []

    enum Status {
        case none
        case approved
        case inProgress
        case issue
    }
}
