import Foundation

@MainActor
extension CADCommandRegistry {
    func registerExtendedScenarioCommands(appState: CADAppState) {
        register(CADCommand(
            id: "sketch.3d",
            titleRU: "3D-эскиз",
            titleEN: "3D Sketch",
            icon: "cube.transparent",
            shortcut: nil,
            workbenches: [.sketch],
            isAvailable: { context in context.workbench == .sketch },
            execute: {
                appState.selectedTool = "sketch3D"
                MirEventBus.shared.publish(.commandRequested("sketch.3d"))
                appState.showNotification(
                    appState.ui.language == .russian ? "3D-эскиз активирован" : "3D Sketch activated",
                    type: .info
                )
            }
        ))

        register(CADCommand(
            id: "assembly.kinematics",
            titleRU: "Кинематика",
            titleEN: "Kinematics",
            icon: "figure.walk.motion",
            shortcut: nil,
            workbenches: [.assembly],
            isAvailable: { context in context.workbench == .assembly },
            execute: {
                appState.switchSubMode(to: .assemblyKinematics)
                MirEventBus.shared.publish(.commandRequested("assembly.kinematics"))
            }
        ))

        register(CADCommand(
            id: "assembly.motion",
            titleRU: "Исследование движения",
            titleEN: "Motion Study",
            icon: "play.rectangle",
            shortcut: nil,
            workbenches: [.assembly],
            isAvailable: { context in context.workbench == .assembly },
            execute: {
                appState.switchSubMode(to: .assemblyMotion)
                MirEventBus.shared.publish(.commandRequested("assembly.motion"))
            }
        ))

        register(CADCommand(
            id: "simulation.multiphysics",
            titleRU: "Мультифизика",
            titleEN: "Multiphysics",
            icon: "circle.hexagongrid",
            shortcut: nil,
            workbenches: [.simulation],
            isAvailable: { context in context.workbench == .simulation },
            execute: {
                appState.setPhysics(.multiphysics)
                MirEventBus.shared.publish(.commandRequested("simulation.multiphysics"))
            }
        ))

        register(CADCommand(
            id: "simulation.thermal",
            titleRU: "Тепловой расчёт",
            titleEN: "Thermal Analysis",
            icon: "thermometer.medium",
            shortcut: nil,
            workbenches: [.simulation],
            isAvailable: { context in context.workbench == .simulation },
            execute: {
                appState.setPhysics(.thermal)
                MirEventBus.shared.publish(.commandRequested("simulation.thermal"))
            }
        ))

        register(CADCommand(
            id: "simulation.fluid",
            titleRU: "Расчёт потоков",
            titleEN: "Fluid Analysis",
            icon: "wind",
            shortcut: nil,
            workbenches: [.simulation],
            isAvailable: { context in context.workbench == .simulation },
            execute: {
                appState.setPhysics(.fluid)
                MirEventBus.shared.publish(.commandRequested("simulation.fluid"))
            }
        ))

        register(CADCommand(
            id: "drawing.section",
            titleRU: "Сечение на чертеже",
            titleEN: "Drawing Section",
            icon: "rectangle.split.2x1",
            shortcut: nil,
            workbenches: [.drawing],
            isAvailable: { context in context.workbench == .drawing },
            execute: { MirEventBus.shared.publish(.commandRequested("drawing.section")) }
        ))

        register(CADCommand(
            id: "drawing.bom",
            titleRU: "Спецификация",
            titleEN: "Bill of Materials",
            icon: "list.bullet.rectangle",
            shortcut: nil,
            workbenches: [.drawing],
            isAvailable: { context in context.workbench == .drawing },
            execute: { MirEventBus.shared.publish(.commandRequested("drawing.bom")) }
        ))

        register(CADCommand(
            id: "collaboration.comment",
            titleRU: "Добавить комментарий",
            titleEN: "Add Comment",
            icon: "text.bubble",
            shortcut: "C",
            workbenches: [.collaboration],
            isAvailable: { context in context.workbench == .collaboration },
            execute: { MirEventBus.shared.publish(.commandRequested("collaboration.comment")) }
        ))

        register(CADCommand(
            id: "collaboration.review",
            titleRU: "Режим ревью",
            titleEN: "Review Mode",
            icon: "checkmark.seal",
            shortcut: nil,
            workbenches: [.collaboration],
            isAvailable: { context in context.workbench == .collaboration },
            execute: {
                appState.switchSubMode(to: .collaborationReview)
                MirEventBus.shared.publish(.commandRequested("collaboration.review"))
            }
        ))

        register(CADCommand(
            id: "visualization.render",
            titleRU: "Визуализировать",
            titleEN: "Render",
            icon: "camera.aperture",
            shortcut: "⌘P",
            workbenches: [.visualization],
            isAvailable: { context in context.workbench == .visualization },
            execute: { MirEventBus.shared.publish(.commandRequested("visualization.render")) }
        ))

        register(CADCommand(
            id: "visualization.camera",
            titleRU: "Камера",
            titleEN: "Camera",
            icon: "camera",
            shortcut: nil,
            workbenches: [.visualization],
            isAvailable: { context in context.workbench == .visualization },
            execute: { appState.selectedTool = "camera" }
        ))
    }
}
