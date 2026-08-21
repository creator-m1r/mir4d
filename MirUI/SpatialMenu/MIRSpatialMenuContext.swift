import Foundation
import MirUIHandGesture

extension MIRSpatialMenuSceneContext {
    var interactionTarget: MIR4DInteractionTarget {
        if sculptActive && (bodySelected || hasSelection) { return .sculpt }
        if faceSelected { return .face }
        if sketchActive { return .sketch }
        if bodySelected || hasSelection { return .object }
        return .empty
    }
}

struct MIRSpatialMenuSceneContext: Equatable, Sendable {
    var workbench: String
    var selectionKind: String
    var hasSelection: Bool
    var sketchActive: Bool
    var faceSelected: Bool
    var bodySelected: Bool
    var sculptActive: Bool

    static let idle = MIRSpatialMenuSceneContext(
        workbench: "model",
        selectionKind: "none",
        hasSelection: false,
        sketchActive: false,
        faceSelected: false,
        bodySelected: false,
        sculptActive: false
    )
}

enum MIRSpatialMenuContext {
    @MainActor
    static func resolve(appState: CADAppState) -> MIRSpatialMenuSceneContext {
        MIRSpatialMenuSceneContext(
            workbench: appState.workbench.rawValue,
            selectionKind: appState.selection.primaryKind.rawValue,
            hasSelection: appState.selection.hasSelection,
            sketchActive: appState.workbench == .sketch,
            faceSelected: appState.selection.primaryKind == .face,
            bodySelected: appState.selection.primaryKind == .body,
            sculptActive: appState.selectedTool == "sculpt"
        )
    }

    static func tree(for context: MIRSpatialMenuSceneContext) -> [MIRSpatialMenuItem] {
        let intents = [
            createIntent(context: context),
            editIntent(context: context),
            objectIntent(context: context),
            viewIntent(),
            measureIntent(),
            sketchIntent(context: context),
            assemblyIntent(),
            projectIntent()
        ]
        return Array(intents.prefix(8))
    }

    private static func createIntent(context: MIRSpatialMenuSceneContext) -> MIRSpatialMenuItem {
        MIRSpatialMenuItem(
            id: "create",
            titleRU: "Создать",
            titleEN: "Create",
            icon: "plus",
            command: "create",
            children: [
                MIRSpatialMenuItem(
                    id: "create.solid",
                    titleRU: "Тело",
                    titleEN: "Solid",
                    icon: "cube.transparent",
                    command: "create.solid",
                    children: [
                        tool("create.solid.extrude", "Выдавить", "Extrude", "cube.fill", "model.extrude"),
                        tool("create.solid.revolve", "Вращение", "Revolve", "rotate.3d", "model.revolve"),
                        tool("create.solid.sweep", "Траектория", "Sweep", "arrow.right.circle", "model.extrude"),
                        tool("create.solid.loft", "Лофт", "Loft", "square.stack.3d.up", "model.extrude"),
                        tool("create.solid.boolean", "Булева", "Boolean", "square.on.square", "model.extrude")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "create.surface",
                    titleRU: "Поверхность",
                    titleEN: "Surface",
                    icon: "square.3d.stack.3d",
                    command: "create.surface",
                    children: [
                        tool("create.surface.extrude", "Выдавить", "Extrude", "cube.fill", "model.extrude"),
                        tool("create.surface.offset", "Смещение", "Offset", "arrow.up.and.down", "modify.offset"),
                        tool("create.surface.fillet", "Скругление", "Fillet", "paintbrush", "modify.fillet"),
                        tool("create.surface.chamfer", "Фаска", "Chamfer", "scribble", "modify.chamfer"),
                        tool("create.surface.sketch", "Эскиз", "Sketch", "pencil.and.ruler", "create.sketch")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "create.sketchcat",
                    titleRU: "Эскиз",
                    titleEN: "Sketch",
                    icon: "pencil.and.ruler",
                    command: "create.sketch",
                    children: [
                        tool("create.sketchcat.line", "Линия", "Line", "line.diagonal", "sketch.line"),
                        tool("create.sketchcat.rect", "Прямоугольник", "Rectangle", "rectangle", "sketch.rectangle"),
                        tool("create.sketchcat.circle", "Окружность", "Circle", "circle", "sketch.circle"),
                        tool("create.sketchcat.arc", "Дуга", "Arc", "arc.starburst", "sketch.arc")
                    ]
                )
            ]
        )
    }

    private static func editIntent(context: MIRSpatialMenuSceneContext) -> MIRSpatialMenuItem {
        let geometryChildren: [MIRSpatialMenuItem]
        if context.faceSelected {

            geometryChildren = [
                tool("edit.face.extrude", "Выдавить", "Extrude", "cube.fill", "model.extrude"),
                tool("edit.face.offset", "Смещение", "Offset", "arrow.up.and.down", "modify.offset"),
                tool("edit.face.fillet", "Скругление", "Fillet", "paintbrush", "modify.fillet"),
                tool("edit.face.chamfer", "Фаска", "Chamfer", "scribble", "modify.chamfer"),
                tool("edit.face.sketch", "Эскиз", "Sketch", "pencil.and.ruler", "create.sketch")
            ]
        } else {
            geometryChildren = [
                tool("edit.geometry.fillet", "Скругление", "Fillet", "paintbrush", "modify.fillet"),
                tool("edit.geometry.chamfer", "Фаска", "Chamfer", "scribble", "modify.chamfer"),
                tool("edit.geometry.offset", "Смещение", "Offset", "arrow.up.and.down", "modify.offset"),
                tool("edit.geometry.split", "Разделить", "Split", "rectangle.split.3x1", "modify.split")
            ]
        }

        return MIRSpatialMenuItem(
            id: "edit",
            titleRU: "Изменить",
            titleEN: "Edit",
            icon: "slider.horizontal.3",
            command: "edit",
            children: [
                MIRSpatialMenuItem(
                    id: "edit.transform",
                    titleRU: "Переместить",
                    titleEN: "Transform",
                    icon: "arrow.up.and.down.and.arrow.left.and.arrow.right",
                    command: "edit.transform",
                    children: [
                        tool("edit.transform.move", "Переместить", "Move", "arrow.up.and.down.and.arrow.left.and.arrow.right", "transform.move"),
                        tool("edit.transform.rotate", "Повернуть", "Rotate", "rotate.right", "transform.rotate"),
                        tool("edit.transform.scale", "Масштаб", "Scale", "arrow.up.left.and.arrow.down.right", "transform.scale"),
                        tool("edit.transform.axis", "Ось", "Axis", "axis.arrow", "transform.axis")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "edit.geometry",
                    titleRU: "Геометрия",
                    titleEN: "Geometry",
                    icon: "wand.and.stars",
                    command: "edit.geometry",
                    children: geometryChildren
                ),
                MIRSpatialMenuItem(
                    id: "edit.history",
                    titleRU: "История",
                    titleEN: "History",
                    icon: "arrow.uturn.backward",
                    command: "edit.history",
                    children: [
                        tool("edit.history.undo", "Отменить", "Undo", "arrow.uturn.backward", "history.undo"),
                        tool("edit.history.redo", "Повторить", "Redo", "arrow.uturn.forward", "history.redo")
                    ]
                )
            ]
        )
    }

    private static func objectIntent(context: MIRSpatialMenuSceneContext) -> MIRSpatialMenuItem {
        let categories: [MIRSpatialMenuItem]
        if context.faceSelected {
            categories = [
                MIRSpatialMenuItem(
                    id: "object.face",
                    titleRU: "Грань",
                    titleEN: "Face",
                    icon: "square.fill",
                    command: "object.face",
                    children: [
                        tool("object.face.extrude", "Выдавить", "Extrude", "cube.fill", "model.extrude"),
                        tool("object.face.offset", "Смещение", "Offset", "arrow.up.and.down", "modify.offset"),
                        tool("object.face.fillet", "Скругление", "Fillet", "paintbrush", "modify.fillet"),
                        tool("object.face.chamfer", "Фаска", "Chamfer", "scribble", "modify.chamfer"),
                        tool("object.face.sketch", "Эскиз", "Sketch", "pencil.and.ruler", "create.sketch")
                    ]
                )
            ]
        } else {
            categories = [
                MIRSpatialMenuItem(
                    id: "object.body",
                    titleRU: "Тело",
                    titleEN: "Body",
                    icon: "cube.transparent",
                    command: "object.body",
                    children: [
                        tool("object.body.sculpt", "Воздушный скульпт", "Air Sculpt", "wand.and.rays", "model.sculpt"),
                        tool("object.body.properties", "Свойства", "Properties", "info.circle", "object.properties"),
                        tool("object.body.material", "Материал", "Material", "paintpalette", "object.material"),
                        tool("object.body.delete", "Удалить", "Delete", "trash", "object.delete"),
                        tool("object.body.duplicate", "Дублировать", "Duplicate", "plus.square.on.square", "object.duplicate")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "object.component",
                    titleRU: "Компонент",
                    titleEN: "Component",
                    icon: "shippingbox",
                    command: "object.component",
                    children: [
                        tool("object.component.insert", "Вставить", "Insert", "plus.square.dashed", "assembly.component"),
                        tool("object.component.mate", "Связать", "Mate", "link", "assembly.mate"),
                        tool("object.component.ground", "Закрепить", "Ground", "pin", "assembly.constraint")
                    ]
                )
            ]
        }

        return MIRSpatialMenuItem(
            id: "object",
            titleRU: "Объект",
            titleEN: "Object",
            icon: "shippingbox",
            command: "object",
            children: categories,
            enabled: context.hasSelection || !context.faceSelected
        )
    }

    private static func viewIntent() -> MIRSpatialMenuItem {
        MIRSpatialMenuItem(
            id: "view",
            titleRU: "Вид",
            titleEN: "View",
            icon: "eye",
            command: "view",
            children: [
                MIRSpatialMenuItem(
                    id: "view.camera",
                    titleRU: "Камера",
                    titleEN: "Camera",
                    icon: "camera",
                    command: "view.camera",
                    children: [
                        tool("view.camera.fit", "Показать всё", "Fit All", "arrow.up.left.and.arrow.down.right", "viewport.fit"),
                        tool("view.camera.iso", "Изометрия", "Isometric", "cube", "view.isometric"),
                        tool("view.camera.ortho", "Проекция", "Orthographic", "square.split.2x2", "view.orthographic")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "view.display",
                    titleRU: "Отображение",
                    titleEN: "Display",
                    icon: "display",
                    command: "view.display",
                    children: [
                        tool("view.display.grid", "Сетка", "Grid", "grid", "viewport.grid"),
                        tool("view.display.axes", "Оси", "Axes", "axis.arrow", "viewport.axes"),
                        tool("view.display.section", "Сечение", "Section", "scissors", "viewport.section")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "view.nav",
                    titleRU: "Навигация",
                    titleEN: "Navigate",
                    icon: "location",
                    command: "view.nav",
                    children: [
                        tool("view.nav.pan", "Панорама", "Pan", "hand.draw", "viewport.pan"),
                        tool("view.nav.zoom", "Масштаб", "Zoom", "plus.magnifyingglass", "viewport.zoom"),
                        tool("view.nav.select", "Выбор", "Select", "cursorarrow", "viewport.select")
                    ]
                )
            ]
        )
    }

    private static func measureIntent() -> MIRSpatialMenuItem {
        MIRSpatialMenuItem(
            id: "measure",
            titleRU: "Измерить",
            titleEN: "Measure",
            icon: "ruler",
            command: "measure",
            children: [
                MIRSpatialMenuItem(
                    id: "measure.distance",
                    titleRU: "Расстояние",
                    titleEN: "Distance",
                    icon: "arrow.left.and.right",
                    command: "measure.distance",
                    children: [
                        tool("measure.distance.edge", "Ребро", "Edge", "line.diagonal", "measure.distance"),
                        tool("measure.distance.center", "Центр", "Center", "target", "measure.distance")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "measure.angle",
                    titleRU: "Угол",
                    titleEN: "Angle",
                    icon: "angle",
                    command: "measure.angle",
                    children: [
                        tool("measure.angle.points", "Три точки", "Three Points", "point.3.connected.trianglepath.dotted", "measure.angle"),
                        tool("measure.angle.edges", "Два ребра", "Two Edges", "line.2.horizontal.decrease.circle", "measure.angle")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "measure.dimension",
                    titleRU: "Размер",
                    titleEN: "Dimension",
                    icon: "ruler",
                    command: "measure.dimension",
                    children: [
                        tool("measure.dimension.h", "Горизонтальный", "Horizontal", "arrow.left.and.right.square", "measure.dimension"),
                        tool("measure.dimension.v", "Вертикальный", "Vertical", "arrow.up.and.down.square", "measure.dimension"),
                        tool("measure.dimension.d", "Диаметр", "Diameter", "circle.dashed", "measure.dimension")
                    ]
                )
            ]
        )
    }

    private static func sketchIntent(context: MIRSpatialMenuSceneContext) -> MIRSpatialMenuItem {
        let categories: [MIRSpatialMenuItem]
        if context.sketchActive {

            categories = [
                MIRSpatialMenuItem(
                    id: "sketch.draw",
                    titleRU: "Рисование",
                    titleEN: "Draw",
                    icon: "pencil",
                    command: "sketch.draw",
                    children: [
                        tool("sketch.draw.line", "Линия", "Line", "line.diagonal", "sketch.line"),
                        tool("sketch.draw.arc", "Дуга", "Arc", "arc.starburst", "sketch.arc"),
                        tool("sketch.draw.circle", "Окружность", "Circle", "circle", "sketch.circle"),
                        tool("sketch.draw.rect", "Прямоугольник", "Rectangle", "rectangle", "sketch.rectangle")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "sketch.modify",
                    titleRU: "Изменение",
                    titleEN: "Modify",
                    icon: "wand.and.stars",
                    command: "sketch.modify",
                    children: [
                        tool("sketch.modify.trim", "Обрезать", "Trim", "scissors", "sketch.trim"),
                        tool("sketch.modify.offset", "Смещение", "Offset", "arrow.up.and.down", "sketch.offset"),
                        tool("sketch.modify.constraint", "Ограничение", "Constraint", "link", "sketch.constraint"),
                        tool("sketch.modify.dimension", "Размер", "Dimension", "ruler", "sketch.dimension")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "sketch.exit",
                    titleRU: "Завершить",
                    titleEN: "Finish",
                    icon: "arrow.forward",
                    command: "sketch.exit",
                    children: [
                        tool("sketch.exit.extrude", "Выдавить", "Extrude", "cube.fill", "model.extrude"),
                        tool("sketch.exit.close", "Закрыть", "Close", "xmark", "sketch.close")
                    ]
                )
            ]
        } else {
            categories = [
                MIRSpatialMenuItem(
                    id: "sketch.new",
                    titleRU: "Новый эскиз",
                    titleEN: "New Sketch",
                    icon: "pencil.and.ruler",
                    command: "sketch.new",
                    children: [
                        tool("sketch.new.xy", "Плоскость XY", "Plane XY", "square.grid.3x3", "create.sketch"),
                        tool("sketch.new.yz", "Плоскость YZ", "Plane YZ", "square.grid.3x3.fill", "create.sketch"),
                        tool("sketch.new.zx", "Плоскость ZX", "Plane ZX", "square.grid.3x3.topleft.filled", "create.sketch")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "sketch.offset",
                    titleRU: "Смещение",
                    titleEN: "Offset",
                    icon: "arrow.up.and.down",
                    command: "sketch.offset",
                    children: [
                        tool("sketch.offset.curve", "Кривая", "Curve", "curlybraces", "sketch.offset")
                    ]
                )
            ]
        }

        return MIRSpatialMenuItem(
            id: "sketch",
            titleRU: "Эскиз",
            titleEN: "Sketch",
            icon: "pencil.and.ruler",
            command: "sketch",
            children: categories
        )
    }

    private static func assemblyIntent() -> MIRSpatialMenuItem {
        MIRSpatialMenuItem(
            id: "assembly",
            titleRU: "Сборка",
            titleEN: "Assembly",
            icon: "square.stack.3d.up",
            command: "assembly",
            children: [
                MIRSpatialMenuItem(
                    id: "assembly.constraint",
                    titleRU: "Связи",
                    titleEN: "Constraints",
                    icon: "link",
                    command: "assembly.constraint",
                    children: [
                        tool("assembly.constraint.mate", "Сопряжение", "Mate", "link", "assembly.mate"),
                        tool("assembly.constraint.interference", "Проверка", "Interference", "exclamationmark.triangle", "assembly.interference")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "assembly.component",
                    titleRU: "Компоненты",
                    titleEN: "Components",
                    icon: "shippingbox",
                    command: "assembly.component",
                    children: [
                        tool("assembly.component.insert", "Вставить", "Insert", "plus.square.dashed", "assembly.component"),
                        tool("assembly.component.move", "Переместить", "Move", "arrow.up.and.down.and.arrow.left.and.arrow.right", "transform.move")
                    ]
                )
            ]
        )
    }

    private static func projectIntent() -> MIRSpatialMenuItem {
        MIRSpatialMenuItem(
            id: "project",
            titleRU: "Проект",
            titleEN: "Project",
            icon: "folder",
            command: "project",
            children: [
                MIRSpatialMenuItem(
                    id: "project.file",
                    titleRU: "Файл",
                    titleEN: "File",
                    icon: "folder",
                    command: "project.file",
                    children: [
                        tool("project.file.new", "Новый", "New", "doc.badge.plus", "document.new"),
                        tool("project.file.open", "Открыть", "Open", "folder", "project.open"),
                        tool("project.file.save", "Сохранить", "Save", "square.and.arrow.down", "file.save"),
                        tool("project.file.export", "Экспорт", "Export", "square.and.arrow.up", "file.export")
                    ]
                ),
                MIRSpatialMenuItem(
                    id: "project.collab",
                    titleRU: "Команда",
                    titleEN: "Team",
                    icon: "person.2",
                    command: "project.collab",
                    children: [
                        tool("project.collab.team", "Чат команды", "Team Chat", "bubble.left.and.bubble.right", "collaboration.chat"),
                        tool("project.collab.review", "Ревью", "Review", "checkmark.seal", "collaboration.review")
                    ]
                )
            ]
        )
    }

    private static func tool(_ id: String, _ titleRU: String, _ titleEN: String, _ icon: String, _ command: String) -> MIRSpatialMenuItem {
        MIRSpatialMenuItem(id: id, titleRU: titleRU, titleEN: titleEN, icon: icon, command: command)
    }
}