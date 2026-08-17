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
    @Published private(set) var currentTime: Double = 0.0
    @Published private(set) var isPlaying: Bool = false
    @Published private(set) var scenarioID: String = "scenario-main"
    @Published private(set) var branchID: String = "branch-main"

    let startTime: Double = 0.0
    let endTime: Double = 10.0
    let tickInterval: TimeInterval = 1.0 / 60.0
    private var timer: Timer?

    init() {}

    func seek(_ time: Double) {
        currentTime = min(max(time, startTime), endTime)
        publishTimeChanged()
    }

    func step(by delta: Double) { seek(currentTime + delta) }

    func play() {
        guard !isPlaying else { return }
        if currentTime >= endTime { currentTime = startTime }
        isPlaying = true
        publishTimeChanged()
        timer?.invalidate()
        timer = Timer.scheduledTimer(withTimeInterval: tickInterval, repeats: true) { [weak self] _ in
            Task { @MainActor [weak self] in
                guard let self else { return }
                let next = self.currentTime + self.tickInterval
                if next >= self.endTime {
                    self.seek(self.endTime)
                    self.pause()
                } else {
                    self.seek(next)
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
        if isPlaying { pause() } else { play() }
    }

    func reset() {
        pause()
        currentTime = startTime
        publishTimeChanged()
    }

    func finish() {
        pause()
        currentTime = endTime
        publishTimeChanged()
    }

    func createScenario() {
        scenarioID = "scenario-\(UUID().uuidString.prefix(8).lowercased())"
        branchID = "branch-main"
        publishTimeChanged()
    }

    func createBranch() {
        branchID = "branch-\(UUID().uuidString.prefix(8).lowercased())"
        publishTimeChanged()
    }

    var snapshot: CADTimeState {
        CADTimeState(current: currentTime, start: startTime, end: endTime, isPlaying: isPlaying, scenarioID: scenarioID, branchID: branchID)
    }

    private func publishTimeChanged() {
        MirEventBus.shared.publish(.timeChanged(snapshot))
    }
}

// MARK: - Notifications

enum NotificationType { case info, success, warning, error }

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
/// Якорь эскиза: плоскость построения и её базис в мировых координатах.
/// `id` совпадает с базовым id плоскости в MirEngine (1=XY, 2=YZ, 3=ZX).
struct SketchPlaneAnchor: Identifiable, Equatable {
    let id: UInt32
    let name: String
    let preset: MirCameraPreset
    let origin: (x: Double, y: Double, z: Double)
    let normal: (x: Double, y: Double, z: Double)
    let xAxis: (x: Double, y: Double, z: Double)
    let yAxis: (x: Double, y: Double, z: Double)

    /// Ручная реализация: кортежи не синтезируют `Equatable`, а `onChange`
    /// требует соответствия протоколу. Плоскость однозначно задаётся `id`.
    static func == (lhs: SketchPlaneAnchor, rhs: SketchPlaneAnchor) -> Bool {
        lhs.id == rhs.id
    }

    /// Три стандартные базовые плоскости (нормаль совпадает с осью, пресет —
    /// вид спереди/справа/сверху, перпендикулярный плоскости).
    static let xy = SketchPlaneAnchor(
        id: 1, name: "XY", preset: .front,
        origin: (0, 0, 0), normal: (0, 0, 1), xAxis: (1, 0, 0), yAxis: (0, 1, 0))
    static let yz = SketchPlaneAnchor(
        id: 2, name: "YZ", preset: .right,
        origin: (0, 0, 0), normal: (1, 0, 0), xAxis: (0, 1, 0), yAxis: (0, 0, 1))
    static let zx = SketchPlaneAnchor(
        id: 3, name: "ZX", preset: .top,
        origin: (0, 0, 0), normal: (0, 1, 0), xAxis: (0, 0, 1), yAxis: (1, 0, 0))

    static let standard: [SketchPlaneAnchor] = [.xy, .yz, .zx]
}

final class CADAppState: ObservableObject {
    @Published var workbench: CADWorkbench = .model
    @Published var subMode: CADSubMode = .modelFeature
    @Published var selectedTool: String = "select"
    @Published var selection = CADSelectionState()
    @Published var simulation = CADSimulationState()
    @Published var interaction = CADInteractionState.forWorkbench(.model)
    @Published var ui = CADUIState()
    @Published var panelState = CADPanelState.forWorkbench(.model)
    @Published var selectedTreeItem: String = "Система жизнеобеспечения"
    @Published var activePropTab: Int = 0
    @Published var notifications: [CADNotification] = []
    @Published var documentName: String = "Новый проект"
    @Published var documentDirty: Bool = false
    @Published var gridVisible: Bool = true
    @Published var axesVisible: Bool = true
    @Published var sectionMode: Bool = false

    /// Выбранная плоскость построения эскиза. `nil` означает, что пользователь
    /// ещё не выбрал плоскость — в этом случае показывается выборщик плоскости.
    @Published var sketchPlane: SketchPlaneAnchor?

    let time = TimeCoordinator()
    private var cancellables = Set<AnyCancellable>()
    private let modelRuntime = MIR4DModelRuntime.shared

    var visiblePanels: Set<CADPanel> { panelState.visible }
    var selectionCount: Int { selection.count }
    var currentTime: Double { time.currentTime }
    var isPlaying: Bool { time.isPlaying }
    var timeState: CADTimeState { time.snapshot }

    var activeContext: CADActiveContext {
        CADActiveContext(workbench: workbench, subMode: subMode, selection: selection, time: timeState, interaction: interaction, simulation: simulation, experience: ui.experience, language: ui.language)
    }

    /// The workspace tree is now derived from the persisted model document.
    /// UI no longer owns a second, unrelated tree identity set.
    var treeData: [TreeNodeData] {
        [makeTreeNode(from: modelRuntime.document.root)]
    }

    init() {
        time.objectWillChange
            .receive(on: RunLoop.main)
            .sink { [weak self] _ in self?.objectWillChange.send() }
            .store(in: &cancellables)

        modelRuntime.objectWillChange
            .receive(on: RunLoop.main)
            .sink { [weak self] _ in self?.objectWillChange.send() }
            .store(in: &cancellables)
    }

    private func makeTreeNode(from node: MIR4DModelNode) -> TreeNodeData {
        TreeNodeData(
            id: node.id,
            name: node.title,
            icon: icon(for: node.kind),
            status: .none,
            children: node.children.map(makeTreeNode(from:))
        )
    }

    private func icon(for kind: MIR4DModelNode.Kind) -> String {
        switch kind {
        case .project: return "cube"
        case .component: return "rectangle.split.3x1"
        case .body: return "cube.transparent"
        case .sketch: return "scribble.variable"
        case .operation: return "gearshape.2"
        case .result: return "checkmark.seal"
        }
    }

    // MARK: Workbench

    func selectWorkbench(_ value: CADWorkbench) {
        workbench = value
        subMode = value.defaultSubMode
        selectedTool = value.defaultTool
        panelState = .forWorkbench(value)
        interaction = .forWorkbench(value)
        activePropTab = 0
        if value == .sketch {
            sketchPlane = nil
        }
        MirEventBus.shared.publish(.workbenchChanged(value))
        MirEventBus.shared.publish(.subModeChanged(subMode))
        showNotification(ui.language == .russian ? "Рабочая среда: \(value.titleRU)" : "Workbench: \(value.titleEN)", type: .success)
    }

    func switchSubMode(to value: CADSubMode) {
        guard value.workbench == workbench else { return }
        subMode = value
        selectedTool = value.defaultTool
        MirEventBus.shared.publish(.subModeChanged(value))
    }

    // MARK: Panels

    func togglePanel(_ panel: CADPanel) { panelState.toggle(panel) }

    func panelPlacement(for panel: CADPanel) -> PanelPlacement { panelState.placement(for: panel) }

    func setPanelPlacement(_ placement: PanelPlacement, for panel: CADPanel) {
        panelState.setPlacement(placement, for: panel)
        let title = ui.language == .russian ? panel.titleRU : panel.titleEN
        let zone = ui.language == .russian ? placement.titleRU : placement.titleEN
        showNotification("\(title): \(zone)", type: .info)
    }

    // MARK: UI

    func toggleLanguage() { ui.language = ui.language == .russian ? .english : .russian }
    func toggleExperience() { ui.experience = ui.experience == .expert ? .beginner : .expert }

    // MARK: Selection

    /// Selection crossing the viewport boundary is normalized here.
    ///
    /// MirEngine/OpenGL reports a runtime object ID, while the CAD document owns
    /// stable UUIDs. Numeric body IDs are therefore resolved through the persisted
    /// model before entering CADAppState. UUID selections from the model tree pass
    /// through unchanged.
    func setSelection(ids: [String], kind: CADSelectionKind) {
        let normalizedIDs = ids.compactMap { rawID -> String? in
            if UUID(uuidString: rawID) != nil { return rawID }
            guard kind == .body, let engineID = UInt64(rawID), engineID > 0 else { return rawID }
            return modelRuntime.document.bodyID(forEngineObjectID: engineID)?.uuidString
        }

        selection = CADSelectionState(ids: normalizedIDs, primaryKind: kind)
        MirEventBus.shared.publish(.selectionChanged(selection))
    }

    func clearSelection() {
        selection = CADSelectionState()
        MirEventBus.shared.publish(.selectionChanged(selection))
    }

    // MARK: Viewport

    func toggleGrid() { gridVisible.toggle() }
    func toggleAxes() { axesVisible.toggle() }
    func toggleSection() { sectionMode.toggle() }

    // MARK: Document

    func newDocument() {
        documentName = "Новый проект"
        documentDirty = false
        selectedTreeItem = "Проект"
        clearSelection()
        simulation = CADSimulationState()
        time.reset()
        MirEventBus.shared.publish(.documentChanged)
        showNotification(ui.language == .russian ? "Создан новый проект" : "New project created", type: .success)
    }

    func markDirty() {
        documentDirty = true
        MirEventBus.shared.publish(.documentChanged)
    }

    // MARK: Import / Export

    func importModel(url: URL) {
        documentDirty = true
        MirEventBus.shared.publish(.commandRequested("document.import"))
        showNotification(ui.language == .russian ? "Импорт: \(url.lastPathComponent)" : "Import: \(url.lastPathComponent)", type: .info)
    }

    func exportModel() {
        MIR4DProjectCommands.shared.exportStep(appState: self)
    }

    func loadModel(url: URL) { importModel(url: url) }

    // MARK: 4D Time

    func seek(_ seconds: Double) { time.seek(seconds) }
    func togglePlayback() { time.togglePlayback() }
    func stepBackward() { time.step(by: -1.0) }
    func stepForward() { time.step(by: 1.0) }
    func resetTime() { time.reset() }
    func finishTime() { time.finish() }

    func createTimeScenario() {
        time.createScenario()
        MirEventBus.shared.publish(.commandRequested("fourD.scenario"))
    }

    func createTimeBranch() {
        time.createBranch()
        MirEventBus.shared.publish(.commandRequested("fourD.branch"))
    }

    // MARK: Simulation

    func setPhysics(_ physics: CADSimulationState.PhysicsType) {
        simulation.physics = physics
        MirEventBus.shared.publish(.simulationChanged(simulation))
    }

    func setSimulationPhase(_ phase: CADSimulationState.Phase) {
        simulation.phase = phase
        MirEventBus.shared.publish(.simulationChanged(simulation))
    }

    func updateSimulationProgress(_ progress: Double) {
        simulation.progress = min(max(progress, 0), 1)
        MirEventBus.shared.publish(.simulationChanged(simulation))
    }

    func finishSimulation(resultSetID: String? = nil) {
        simulation.isRunning = false
        simulation.progress = 1
        simulation.resultSetID = resultSetID ?? "result-\(UUID().uuidString.prefix(8).lowercased())"
        simulation.phase = .results
        simulation.solverStatus = ui.language == .russian ? "Расчёт завершён" : "Solve completed"
        MirEventBus.shared.publish(.simulationChanged(simulation))
    }

    func runSimulation() {
        let ru = ui.language == .russian
        simulation.phase = .solve
        simulation.isRunning = true
        simulation.progress = 0
        simulation.solverStatus = ru ? "Расчёт выполняется…" : "Solving…"
        simulation.lastReport = nil
        simulation.lastPassed = nil
        MirEventBus.shared.publish(.simulationChanged(simulation))

        let definition = caeDefinition(for: simulation.physics)
        guard let report = MIR4DRunCAECampaign(definition: definition) else {
            simulation.isRunning = false
            simulation.progress = 0
            simulation.solverStatus = ru ? "CAE-движок недоступен" : "CAE engine unavailable"
            MirEventBus.shared.publish(.simulationChanged(simulation))
            showNotification(ru ? "CAE-движок недоступен" : "CAE engine unavailable", type: .error)
            return
        }

        let passed = ((try? JSONSerialization.jsonObject(with: Data(report.utf8), options: [])) as? [String: Any])?["passed"] as? Bool ?? false

        simulation.lastReport = report
        simulation.lastPassed = passed
        simulation.isRunning = false
        simulation.progress = 1
        simulation.resultSetID = "cae-\(UUID().uuidString.prefix(8).lowercased())"
        simulation.phase = .results
        simulation.solverStatus = passed ? (ru ? "Пройдено" : "Passed") : (ru ? "Не пройдено" : "Failed")
        MirEventBus.shared.publish(.simulationChanged(simulation))
        showNotification(ru ? "Испытание завершено: \(simulation.solverStatus)" : "Test finished: \(simulation.solverStatus)", type: passed ? .success : .warning)
    }

    private func criterionLine(for physics: CADSimulationState.PhysicsType) -> String {
        switch physics {
        case .structural, .multiphysics:
            return "criterion stress 0 1e9"
        case .thermal:
            return "criterion temperature 0 400"
        case .fluid:
            return "criterion density 500 2000"
        case .acoustic:
            return "criterion acoustic 0 1"
        case .chemical:
            return "criterion composition 0 1"
        case .electromagnetic:
            return "criterion temperature 0 400"
        }
    }

    private func caeDefinition(for physics: CADSimulationState.PhysicsType) -> String {
        let criterion = criterionLine(for: physics)
        var body = """
        case specimen
          material temperature 350
          initial flowRate 5
          initial composition reactant 1
          \(criterion)
        """
        if simulation.useGeometry {
            body += "  load fixed 1\n"
        }
        return body
    }

    func fetchSelectedGeometry() {
        let ru = ui.language == .russian
        guard let viewport = MIR4DModelRuntime.shared.viewport else {
            showNotification(ru ? "Вьюпорт недоступен" : "Viewport unavailable", type: .warning)
            return
        }
        var buffer = [CChar](repeating: 0, count: 1024)
        guard MirEngineGetSelectedObjectMetrics(viewport, &buffer, buffer.count) else {
            showNotification(ru ? "Не удалось получить геометрию" : "Could not read geometry", type: .error)
            return
        }
        let json = String(cString: buffer)
        guard let data = json.data(using: .utf8),
              let dict = (try? JSONSerialization.jsonObject(with: data, options: [])) as? [String: Any],
              (dict["hasGeometry"] as? Bool) == true else {
            simulation.geometry = nil
            showNotification(ru ? "Объект не выбран" : "No object selected", type: .warning)
            return
        }
        let geo = CADObjectMetrics(
            objectId: (dict["objectId"] as? NSNumber)?.uint64Value ?? 0,
            sizeX: dict["sizeX"] as? Double ?? 0,
            sizeY: dict["sizeY"] as? Double ?? 0,
            sizeZ: dict["sizeZ"] as? Double ?? 0,
            volume: dict["volume"] as? Double ?? 0,
            surfaceArea: dict["surfaceArea"] as? Double ?? 0,
            vertexCount: dict["vertexCount"] as? Int ?? 0,
            faceCount: dict["faceCount"] as? Int ?? 0
        )
        simulation.geometry = geo
        simulation.useGeometry = true
        MirEventBus.shared.publish(.simulationChanged(simulation))
        showNotification(ru ? "Геометрия: V=\(String(format: "%.3g", geo.volume))" : "Geometry: V=\(String(format: "%.3g", geo.volume))", type: .success)
    }

    func runSweep(parameter: CAESweepParameter, from: Double, to: Double, steps: Int) {
        let ru = ui.language == .russian
        let count = max(1, min(steps, 200))
        let definition = sweepDefinition(parameter: parameter, from: from, to: to, steps: count)
        simulation.phase = .solve
        simulation.isRunning = true
        simulation.progress = 0
        simulation.solverStatus = ru ? "Параметрический прогон…" : "Parametric sweep…"
        simulation.sweepResults = []
        simulation.sweepParameter = parameter
        simulation.sweepFrom = from
        simulation.sweepTo = to
        simulation.sweepSteps = count
        simulation.compareA = nil
        simulation.compareB = nil
        MirEventBus.shared.publish(.simulationChanged(simulation))

        guard let report = MIR4DRunCAECampaign(definition: definition) else {
            simulation.isRunning = false
            simulation.progress = 0
            simulation.solverStatus = ru ? "CAE-движок недоступен" : "CAE engine unavailable"
            MirEventBus.shared.publish(.simulationChanged(simulation))
            showNotification(ru ? "CAE-движок недоступен" : "CAE engine unavailable", type: .error)
            return
        }

        let rows = parseSweepRows(from: report, from: from, to: to, steps: count)
        simulation.sweepResults = rows
        simulation.lastReport = report
        simulation.isRunning = false
        simulation.progress = 1
        simulation.phase = .results
        let passedCount = rows.filter { $0.passed }.count
        simulation.lastPassed = rows.allSatisfy { $0.passed }
        simulation.solverStatus = ru
            ? "Прогон: \(passedCount)/\(rows.count) пройдено"
            : "Sweep: \(passedCount)/\(rows.count) passed"
        MirEventBus.shared.publish(.simulationChanged(simulation))
        showNotification(ru ? "Прогон завершён: \(passedCount)/\(rows.count)" : "Sweep done: \(passedCount)/\(rows.count)", type: passedCount == rows.count ? .success : .warning)
    }

    func setCompareA(_ row: Int?) {
        simulation.compareA = row
        MirEventBus.shared.publish(.simulationChanged(simulation))
    }

    func setCompareB(_ row: Int?) {
        simulation.compareB = row
        MirEventBus.shared.publish(.simulationChanged(simulation))
    }

    private func fmt(_ value: Double) -> String {
        String(format: "%.4g", value)
    }

    private func sweepDefinition(parameter: CAESweepParameter, from: Double, to: Double, steps: Int) -> String {
        var lines: [String] = []
        for i in 0..<steps {
            let t = steps > 1 ? Double(i) / Double(steps - 1) : 0
            let raw = from + (to - from) * t
            var value = raw
            if parameter == .load, simulation.useGeometry, let geo = simulation.geometry, geo.surfaceArea > 0 {
                // raw интерпретируется как приложенная сила (Н); переводим в давление через площадь
                value = raw / geo.surfaceArea
            }
            var body = """
            case \(parameter.rawValue)_\(i)
              \(parameter.commandTemplate) \(fmt(value))
              initial flowRate 5
              initial composition reactant 1
              \(criterionLine(for: simulation.physics))
            """
            if simulation.useGeometry {
                body += "  load fixed 1\n"
            }
            lines.append(body)
        }
        return lines.joined(separator: "\n")
    }

    private func parseSweepRows(from report: String, from: Double, to: Double, steps: Int) -> [CAESweepRow] {
        guard let dict = (try? JSONSerialization.jsonObject(with: Data(report.utf8), options: [])) as? [String: Any],
              let cases = dict["cases"] as? [[String: Any]] else { return [] }
        var rows: [CAESweepRow] = []
        for (i, item) in cases.enumerated() {
            let t = steps > 1 ? Double(i) / Double(steps - 1) : 0
            let paramValue = from + (to - from) * t
            let name = item["name"] as? String ?? "case_\(i)"
            let passed = item["passed"] as? Bool ?? false
            var metrics: [String: CAETelemetryMetric] = [:]
            if let result = item["result"] as? [String: Any],
               let telemetry = result["telemetry"] as? [String: Any] {
                for (key, val) in telemetry {
                    if let m = val as? [String: Any],
                       let mn = m["min"] as? Double,
                       let mx = m["max"] as? Double {
                        metrics[key] = CAETelemetryMetric(min: mn, max: mx)
                    }
                }
            }
            rows.append(CAESweepRow(index: i, parameterValue: paramValue, caseName: name, passed: passed, metrics: metrics))
        }
        return rows
    }

    // MARK: Notifications

    func showNotification(_ msg: String, type: NotificationType = .info) {
        let notification = CADNotification(message: msg, type: type)
        notifications.append(notification)
        Task { @MainActor [weak self] in
            try? await Task.sleep(for: .seconds(3.5))
            guard let self else { return }
            notifications.removeAll { $0.id == notification.id }
        }
    }
}

// MARK: - Project Tree Node

struct TreeNodeData: Identifiable {
    let id: UUID
    let name: String
    let icon: String
    var status: Status = .none
    var children: [TreeNodeData] = []

    enum Status {
        case none
        case approved
        case inProgress
        case issue
    }
}
