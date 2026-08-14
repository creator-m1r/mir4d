import Foundation

// MARK: - Workbenches

/// Official MIR 4D workbench hierarchy.
/// Analysis is intentionally represented by Simulation.PhysicsType,
/// not as a separate workbench.
enum CADWorkbench: String, CaseIterable, Identifiable, Hashable {
    case model
    case sketch
    case assembly
    case simulation
    case fourD
    case drawing
    case collaboration
    case visualization

    var id: String { rawValue }

    var titleRU: String {
        switch self {
        case .model: return "Модель"
        case .sketch: return "Эскиз"
        case .assembly: return "Сборка"
        case .simulation: return "Симуляция"
        case .fourD: return "4D"
        case .drawing: return "Чертёж"
        case .collaboration: return "Сотрудничество"
        case .visualization: return "Визуализация"
        }
    }

    var titleEN: String {
        switch self {
        case .model: return "Model"
        case .sketch: return "Sketch"
        case .assembly: return "Assembly"
        case .simulation: return "Simulation"
        case .fourD: return "4D"
        case .drawing: return "Drawing"
        case .collaboration: return "Collaboration"
        case .visualization: return "Visualization"
        }
    }

    var icon: String {
        switch self {
        case .model: return "cube"
        case .sketch: return "pencil.and.ruler"
        case .assembly: return "square.stack.3d.up"
        case .simulation: return "waveform.path.ecg"
        case .fourD: return "clock.arrow.circlepath"
        case .drawing: return "doc.text"
        case .collaboration: return "person.2"
        case .visualization: return "camera.aperture"
        }
    }

    var defaultSubMode: CADSubMode {
        switch self {
        case .model: return .modelFeature
        case .sketch: return .sketchEdit
        case .assembly: return .assemblyStructure
        case .simulation: return .simulationSetup
        case .fourD: return .fourDScrub
        case .drawing: return .drawingDocument
        case .collaboration: return .collaborationReview
        case .visualization: return .visualizationScene
        }
    }

    var defaultTool: String {
        switch self {
        case .model: return "select"
        case .sketch: return "sketch"
        case .assembly: return "select"
        case .simulation: return "select"
        case .fourD: return "scrub"
        case .drawing: return "select"
        case .collaboration: return "select"
        case .visualization: return "select"
        }
    }

    func supports(_ subMode: CADSubMode) -> Bool {
        subMode.workbench == self
    }
}

// MARK: - SubModes

enum CADSubMode: String, CaseIterable, Identifiable, Hashable {
    case modelFeature
    case modelDirectEdit
    case modelSurface
    case modelMesh

    case sketchCreate
    case sketchEdit
    case sketchConstraint
    case sketchDimension

    case assemblyStructure
    case assemblyMate
    case assemblyKinematics
    case assemblyMotion

    case simulationSetup
    case simulationSolve
    case simulationResults
    case simulationCompare

    case fourDScrub
    case fourDScenario
    case fourDBranch
    case fourDCompare
    case fourDWhatIf

    case drawingDocument
    case collaborationReview
    case visualizationScene

    var id: String { rawValue }

    var workbench: CADWorkbench {
        switch self {
        case .modelFeature, .modelDirectEdit, .modelSurface, .modelMesh:
            return .model
        case .sketchCreate, .sketchEdit, .sketchConstraint, .sketchDimension:
            return .sketch
        case .assemblyStructure, .assemblyMate, .assemblyKinematics, .assemblyMotion:
            return .assembly
        case .simulationSetup, .simulationSolve, .simulationResults, .simulationCompare:
            return .simulation
        case .fourDScrub, .fourDScenario, .fourDBranch, .fourDCompare, .fourDWhatIf:
            return .fourD
        case .drawingDocument:
            return .drawing
        case .collaborationReview:
            return .collaboration
        case .visualizationScene:
            return .visualization
        }
    }

    var titleRU: String {
        switch self {
        case .modelFeature: return "Функции"
        case .modelDirectEdit: return "Прямое редактирование"
        case .modelSurface: return "Поверхности"
        case .modelMesh: return "Сетка"
        case .sketchCreate: return "Создание"
        case .sketchEdit: return "Редактирование"
        case .sketchConstraint: return "Ограничения"
        case .sketchDimension: return "Размеры"
        case .assemblyStructure: return "Структура"
        case .assemblyMate: return "Связи"
        case .assemblyKinematics: return "Кинематика"
        case .assemblyMotion: return "Движение"
        case .simulationSetup: return "Настройка"
        case .simulationSolve: return "Расчёт"
        case .simulationResults: return "Результаты"
        case .simulationCompare: return "Сравнение"
        case .fourDScrub: return "Просмотр времени"
        case .fourDScenario: return "Сценарий"
        case .fourDBranch: return "Ветка"
        case .fourDCompare: return "Сравнение"
        case .fourDWhatIf: return "Что если"
        case .drawingDocument: return "Чертёж"
        case .collaborationReview: return "Ревью"
        case .visualizationScene: return "Сцена"
        }
    }

    var titleEN: String {
        switch self {
        case .modelFeature: return "Feature"
        case .modelDirectEdit: return "Direct Edit"
        case .modelSurface: return "Surface"
        case .modelMesh: return "Mesh"
        case .sketchCreate: return "Create"
        case .sketchEdit: return "Edit"
        case .sketchConstraint: return "Constraint"
        case .sketchDimension: return "Dimension"
        case .assemblyStructure: return "Structure"
        case .assemblyMate: return "Mate"
        case .assemblyKinematics: return "Kinematics"
        case .assemblyMotion: return "Motion"
        case .simulationSetup: return "Setup"
        case .simulationSolve: return "Solve"
        case .simulationResults: return "Results"
        case .simulationCompare: return "Compare"
        case .fourDScrub: return "Scrub"
        case .fourDScenario: return "Scenario"
        case .fourDBranch: return "Branch"
        case .fourDCompare: return "Compare"
        case .fourDWhatIf: return "What-if"
        case .drawingDocument: return "Drawing"
        case .collaborationReview: return "Review"
        case .visualizationScene: return "Scene"
        }
    }

    var defaultTool: String {
        switch self {
        case .sketchCreate: return "point"
        case .sketchEdit: return "select"
        case .sketchConstraint: return "constraint"
        case .sketchDimension: return "dimension"
        case .fourDScrub: return "scrub"
        case .simulationSolve: return "solve"
        default: return "select"
        }
    }
}

// MARK: - Panels

enum CADPanel: String, CaseIterable, Identifiable, Hashable {
    case project
    case properties
    case timeline
    case simulation
    case history
    case aiInspector

    var id: String { rawValue }

    var titleRU: String {
        switch self {
        case .project: return "Проект"
        case .properties: return "Инспектор"
        case .timeline: return "Время"
        case .simulation: return "Симуляция"
        case .history: return "История"
        case .aiInspector: return "AI Inspector"
        }
    }

    var titleEN: String {
        switch self {
        case .project: return "Project"
        case .properties: return "Inspector"
        case .timeline: return "Timeline"
        case .simulation: return "Simulation"
        case .history: return "History"
        case .aiInspector: return "AI Inspector"
        }
    }

    static func defaults(for workbench: CADWorkbench) -> Set<CADPanel> {
        switch workbench {
        case .model, .sketch, .assembly:
            return [.project, .properties]
        case .simulation:
            return [.properties, .timeline, .simulation]
        case .fourD:
            return [.project, .properties, .timeline]
        case .drawing:
            return [.project, .properties]
        case .collaboration:
            return [.project, .properties]
        case .visualization:
            return [.properties]
        }
    }
}

// MARK: - Panel placement

/// Dock zone of a user-movable panel.
enum PanelPlacement: String, CaseIterable, Identifiable, Hashable {
    case left
    case right
    case bottom

    var id: String { rawValue }

    var titleRU: String {
        switch self {
        case .left: return "Слева"
        case .right: return "Справа"
        case .bottom: return "Снизу"
        }
    }

    var titleEN: String {
        switch self {
        case .left: return "Left"
        case .right: return "Right"
        case .bottom: return "Bottom"
        }
    }

    var icon: String {
        switch self {
        case .left: return "sidebar.left"
        case .right: return "sidebar.right"
        case .bottom: return "rectangle.split.3x1"
        }
    }
}

struct CADPanelState: Equatable {
    var visible: Set<CADPanel>
    var placements: [CADPanel: PanelPlacement]

    static func defaultPlacement(for panel: CADPanel) -> PanelPlacement {
        switch panel {
        case .project: return .left
        case .properties: return .right
        case .timeline, .simulation, .history: return .bottom
        case .aiInspector: return .right
        }
    }

    static func defaultPlacements() -> [CADPanel: PanelPlacement] {
        Dictionary(uniqueKeysWithValues: CADPanel.allCases.map { ($0, defaultPlacement(for: $0)) })
    }

    func placement(for panel: CADPanel) -> PanelPlacement {
        placements[panel] ?? Self.defaultPlacement(for: panel)
    }

    mutating func setPlacement(_ placement: PanelPlacement, for panel: CADPanel) {
        placements[panel] = placement
    }

    static func forWorkbench(_ workbench: CADWorkbench) -> Self {
        Self(visible: CADPanel.defaults(for: workbench), placements: defaultPlacements())
    }

    mutating func toggle(_ panel: CADPanel) {
        if visible.contains(panel) {
            visible.remove(panel)
        } else {
            visible.insert(panel)
        }
    }
}

// MARK: - Selection

enum CADSelectionKind: String, Hashable {
    case none
    case vertex
    case edge
    case face
    case body
    case feature
    case sketch
    case component
    case simulationResult
    case drawingView
    case unknown
}

struct CADSelectionState: Equatable {
    var ids: [String] = []
    var primaryKind: CADSelectionKind = .none

    var count: Int { ids.count }
    var hasSelection: Bool { !ids.isEmpty }
    var isMultiSelection: Bool { ids.count > 1 }
}

// MARK: - Time / 4D

struct CADTimeState: Equatable {
    var current: Double = 0
    var start: Double = 0
    var end: Double = 10
    var isPlaying: Bool = false
    var scenarioID: String = "scenario-main"
    var branchID: String = "branch-main"

    var normalizedProgress: Double {
        guard end > start else { return 0 }
        return min(max((current - start) / (end - start), 0), 1)
    }
}

// MARK: - Simulation

struct CADSimulationState: Equatable {
    enum PhysicsType: String, CaseIterable, Identifiable, Hashable {
        case structural
        case thermal
        case fluid
        case electromagnetic
        case acoustic
        case chemical
        case multiphysics

        var id: String { rawValue }

        var titleRU: String {
            switch self {
            case .structural: return "Прочность"
            case .thermal: return "Тепловая"
            case .fluid: return "Потоки"
            case .electromagnetic: return "Электромагнитная"
            case .acoustic: return "Акустическая"
            case .chemical: return "Химическая"
            case .multiphysics: return "Мультифизика"
            }
        }

        var titleEN: String {
            switch self {
            case .structural: return "Structural"
            case .thermal: return "Thermal"
            case .fluid: return "Fluid"
            case .electromagnetic: return "Electromagnetic"
            case .acoustic: return "Acoustic"
            case .chemical: return "Chemical"
            case .multiphysics: return "Multiphysics"
            }
        }
    }

    enum Phase: String, CaseIterable, Identifiable, Hashable {
        case setup
        case solve
        case results
        case compare

        var id: String { rawValue }
    }

    var physics: PhysicsType = .structural
    var phase: Phase = .setup
    var isRunning = false
    var progress: Double = 0
    var solverStatus: String = "Готов"
    var resultSetID: String?
}

// MARK: - Interaction

struct CADInteractionState: Equatable {
    var isDragging = false
    var isSelecting = false
    var isTransforming = false
    var isPreviewing = false
    var snapEnabled = true
    var inferenceEnabled = true
    var gizmoEnabled = true
    var cursorX: Double = 0
    var cursorY: Double = 0
    var snapTargetID: String?
    var activeOperationID: String?

    static func forWorkbench(_ workbench: CADWorkbench) -> Self {
        switch workbench {
        case .model:
            return Self(snapEnabled: false, inferenceEnabled: false, gizmoEnabled: true)
        case .sketch:
            return Self(snapEnabled: true, inferenceEnabled: true, gizmoEnabled: false)
        case .assembly:
            return Self(snapEnabled: true, inferenceEnabled: true, gizmoEnabled: true)
        case .simulation:
            return Self(snapEnabled: false, inferenceEnabled: false, gizmoEnabled: false)
        case .fourD:
            return Self(snapEnabled: false, inferenceEnabled: false, gizmoEnabled: true)
        case .drawing:
            return Self(snapEnabled: true, inferenceEnabled: true, gizmoEnabled: false)
        case .collaboration:
            return Self(snapEnabled: false, inferenceEnabled: false, gizmoEnabled: false)
        case .visualization:
            return Self(snapEnabled: false, inferenceEnabled: false, gizmoEnabled: true)
        }
    }
}

// MARK: - UI state

enum CADLanguage: String, CaseIterable, Identifiable, Hashable {
    case russian = "RU"
    case english = "EN"

    var id: String { rawValue }
}

enum CADExperience: String, CaseIterable, Identifiable, Hashable {
    case beginner
    case expert

    var id: String { rawValue }
}

struct CADUIState: Equatable {
    var language: CADLanguage = .russian
    var experience: CADExperience = .expert
    var commandPalettePresented = false
}

// MARK: - Active context

struct CADActiveContext: Equatable {
    let workbench: CADWorkbench
    let subMode: CADSubMode
    let selection: CADSelectionState
    let time: CADTimeState
    let interaction: CADInteractionState
    let simulation: CADSimulationState
    let experience: CADExperience
    let language: CADLanguage
}
