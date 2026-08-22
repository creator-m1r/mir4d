import SwiftUI

/// Fast interactive sketch workspace.
/// Geometry remains UI-preview data until committed through the engine command bridge.
struct SketchWorkspaceView: View {
    @StateObject private var model = SketchSessionModel()
    @State private var activeTool: SketchTool = .select
    @State private var showGrid = true
    @State private var showConstraints = true
    @State private var snapEnabled = true
    @State private var status = "Эскиз готов к построению"
    @State private var firstPoint: CGPoint?
    /// Накопленные точки для построения сплайна (инструмент .spline).
    @State private var pendingSpline: [CGPoint] = []
    @State private var cursor: CGPoint = .zero
    @State private var scale: CGFloat = 1.0
    @State private var pan: CGSize = .zero
    @State private var extrudeDepth: Double = 10
    /// Число копий для команд массива.
    @State private var patternCount: Int = 3
    /// Расстояние смещения для команды «Сместить».
    @State private var offsetDistance: Double = 10
    /// Тип связи, ожидающей применения. Пока не nil, клики по геометрии
    /// накапливаются в выделение, а после нужного числа — применяются.
    @State private var constraintType: MirEngineSketchConstraint? = nil
    /// Числовое значение для размерных связей (расстояние/угол/радиус/диаметр).
    @State private var constraintValue: String = ""
    /// Вызывается по завершении эскиза (аналог «Готово» старого редактора).
    var onFinish: (() -> Void)? = nil

    private var selectedIDs: Set<UInt32> { Set(model.selectedIDs) }

    var body: some View {
        VStack(spacing: 0) {
            sketchToolbar
            Divider()

            HStack(spacing: 0) {
                sketchToolbox
                Divider()
                sketchCanvas
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                Divider()
                constraintPanel
            }

            Divider()
            statusBar
        }
        .background(Color.black.opacity(0.97))
        .onKeyPress(phases: .down) { press in
            switch press.key {
            case .delete, .deleteForward:
                deleteSelected()
                return .handled
            case .escape:
                firstPoint = nil
                pendingSpline.removeAll()
                model.clearSelection()
                constraintType = nil
                return .handled
            default:
                return .ignored
            }
        }
    }

    private var sketchToolbar: some View {
        HStack(spacing: 7) {
            Label("ЭСКИЗ", systemImage: "pencil.and.ruler")
                .font(.headline)
                .padding(.horizontal, 10)

            toolButton("Выбор", .select)
            toolButton("Линия", .line)
            toolButton("Дуга", .arc)
            toolButton("Окружность", .circle)
            toolButton("Прямоугольник", .rectangle)
            toolButton("Обрезать", .trim)
            toolButton("Размер", .dimension)

            Divider()

            Picker("Связь", selection: $constraintType) {
                Text("нет").tag(MirEngineSketchConstraint?.none)
                ForEach(
                    [MirEngineSketchConstraint.coincident,
                     .horizontal, .vertical, .equal, .parallel, .perpendicular,
                     .distance, .angle, .radius, .diameter],
                    id: \.self
                ) { t in
                    Text(t.title).tag(Optional(t))
                }
            }
            .controlSize(.small)
            .help("Выберите связь, затем отметьте геометрию на холсте")

            if let type = constraintType, type.needsValue {
                TextField("Значение", text: $constraintValue)
                    .frame(width: 72)
                    .controlSize(.small)
                    .help("Числовое значение связи")
            }

            Spacer()

            Toggle("Сетка", isOn: $showGrid)
                .toggleStyle(.checkbox)
            Toggle("Привязки", isOn: $snapEnabled)
                .toggleStyle(.checkbox)
            Toggle("Связи", isOn: $showConstraints)
                .toggleStyle(.checkbox)

            Button("Отменить", action: { if !model.undo() { status = "Нечего отменять" } })
                .buttonStyle(.bordered)
                .disabled(!model.canUndo)
            Button("Вернуть", action: { if !model.redo() { status = "Нечего возвращать" } })
                .buttonStyle(.bordered)
                .disabled(!model.canRedo)

            Button("Удалить", role: .destructive) {
                deleteSelected()
            }
            .buttonStyle(.bordered)
            .disabled(model.selectedIDs.isEmpty)

            Button("Зеркало") {
                model.mirrorSelectionX()
                status = "Отражение выполнено"
            }
            .buttonStyle(.bordered)
            .disabled(model.selectedIDs.isEmpty)

            Divider()

            Stepper(value: $patternCount, in: 2...20) {
                Text("Копий: \(patternCount)")
            }
            .frame(width: 130)

            Button("Массив →") {
                model.patternLinear(count: patternCount, dx: 30, dy: 0)
                status = "Линейный массив: \(patternCount) копий"
            }
            .buttonStyle(.bordered)
            .disabled(model.selectedIDs.isEmpty)

            Button("Массив ↻") {
                model.patternCircular(count: patternCount, center: model.selectionCenter(), angleDegrees: 360.0 / Double(patternCount))
                status = "Круговой массив: \(patternCount) копий"
            }
            .buttonStyle(.bordered)
            .disabled(model.selectedIDs.isEmpty)

            Button("Очистить") {
                model.clearAll()
                model.clearSelection()
                firstPoint = nil
                status = "Эскиз очищен"
            }
            .buttonStyle(.bordered)

            Button("Завершить эскиз") {
                firstPoint = nil
                model.clearSelection()
                activeTool = .select
                status = "Эскиз завершён · геометрия: \(model.geometryCountValue()), связей: \(model.constraintCountValue())"
                onFinish?()
            }
            .buttonStyle(.borderedProminent)

            Divider()

            Stepper(value: $extrudeDepth, in: 1...500) {
                Text("Глубина: \(Int(extrudeDepth))")
            }
            Button("Выдавить") {
                guard let renderer = MIR4DModelRuntime.shared.renderer else {
                    status = "Нет 3D-вида для выдавливания"
                    return
                }
                let id = model.extrude(distance: extrudeDepth, viewport: renderer)
                status = id != 0
                    ? "Выдавлено · объект \(id)"
                    : "Нет профиля для выдавливания"
            }
            .buttonStyle(.borderedProminent)
            .disabled(!model.hasValidProfile)
        }
        .padding(8)
    }

    private var sketchToolbox: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("ГЕОМЕТРИЯ").font(.caption.bold()).foregroundStyle(.secondary)
            toolButton("Выбор", .select)
            toolButton("Линия", .line)
            toolButton("Дуга", .arc)
            toolButton("Окружность", .circle)
            toolButton("Прямоугольник", .rectangle)
            toolButton("Сплайн", .spline)
            if activeTool == .spline && pendingSpline.count >= 2 {
                Button("Готово сплайн") { finishSpline() }
                    .buttonStyle(.borderedProminent)
            }

            Divider().padding(.vertical, 4)
            Text("ОПЕРАЦИИ").font(.caption.bold()).foregroundStyle(.secondary)
            toolButton("Обрезать", .trim)
            toolButton("Смещение", .offset)
            Stepper(value: $offsetDistance, in: -500...500) {
                Text("Смещ: \(Int(offsetDistance))")
            }
            toolButton("Зеркало", .mirror)
            toolButton("Массив", .pattern)
            toolButton("Размер", .dimension)

            Spacer()
            Text("ЛКМ — построение").font(.caption2).foregroundStyle(.secondary)
            Text("Колесо — масштаб").font(.caption2).foregroundStyle(.secondary)
            Text("ПКМ — панорама").font(.caption2).foregroundStyle(.secondary)
        }
        .frame(width: 160)
        .padding(10)
    }

    private var sketchCanvas: some View {
        GeometryReader { proxy in
            ZStack {
                Color.black

                if showGrid {
                    SketchGridView(scale: scale, pan: pan)
                }

                coordinateAxes(in: proxy.size)

                ForEach(0..<model.geometryCountValue(), id: \.self) { index in
                    geometryView(index: UInt32(index), size: proxy.size)
                }

                if showConstraints {
                    ForEach(0..<model.constraintCountValue(), id: \.self) { index in
                        constraintView(index: UInt32(index), size: proxy.size)
                    }
                }

                if let start = firstPoint {
                    previewShape(from: start, size: proxy.size)
                }

                if !pendingSpline.isEmpty {
                    let pts = pendingSpline.map { worldToScreen($0, in: proxy.size) }
                    Path { path in
                        guard let first = pts.first else { return }
                        path.move(to: first)
                        for p in pts.dropFirst() { path.addLine(to: p) }
                    }
                    .stroke(Color.green, lineWidth: 2)
                }

                VStack {
                    HStack {
                        Text(activeTool.title)
                            .font(.caption.weight(.semibold))
                            .padding(.horizontal, 9)
                            .padding(.vertical, 6)
                            .background(.ultraThinMaterial)
                            .clipShape(Capsule())
                        Spacer()
                        Text("\(Int(cursor.x)) : \(Int(cursor.y))")
                            .font(.caption.monospacedDigit())
                            .foregroundStyle(.secondary)
                    }
                    .padding(12)
                    Spacer()
                }
            }
            .contentShape(Rectangle())
            .gesture(canvasGesture(proxy.size))
            .onContinuousHover { phase in
                if case .active(let location) = phase { cursor = location }
            }
        }
    }

    private func canvasGesture(_ size: CGSize) -> some Gesture {
        DragGesture(minimumDistance: 0)
            .onChanged { value in
                cursor = value.location
            }
            .onEnded { value in
                handleCanvasClick(value.location, in: size)
            }
    }

    private func handleCanvasClick(_ screen: CGPoint, in size: CGSize) {
        let world = screenToWorld(screen, in: size)
        let snapped = snap(world)

        switch activeTool {
        case .line:
            if let start = firstPoint {
                let snapped = applyOrtho(snap(world), from: start)
                let result = model.createLine(from: start, to: snapped)
                firstPoint = nil
                if let result, result.success {
                    status = "Линия создана · ID \(result.geometryID) · DOF \(model.degreesOfFreedom.map(String.init) ?? "—")"
                } else {
                    status = "Не удалось создать линию"
                }
            } else {
                firstPoint = snapped
                status = "Первая точка линии выбрана"
            }
        case .circle:
            if let center = firstPoint {
                let radius = distance(center, snapped)
                guard radius > 2 / scale else { firstPoint = nil; return }
                let id = model.createCircle(center: center, radius: Double(radius / scale))
                firstPoint = nil
                status = id != 0 ? "Окружность создана · R \(Int(radius / scale)) · DOF \(model.degreesOfFreedom.map(String.init) ?? "—")" : "Не удалось создать окружность"
            } else {
                firstPoint = snapped
                status = "Центр окружности выбран"
            }
        case .rectangle:
            if let start = firstPoint {
                let id = model.createRectangle(from: start, to: snapped)
                firstPoint = nil
                status = id != 0 ? "Прямоугольник создан (4 линии + связи) · DOF \(model.degreesOfFreedom.map(String.init) ?? "—")" : "Не удалось создать прямоугольник"
            } else {
                firstPoint = snapped
                status = "Первый угол выбран"
            }
        case .arc:
            if let center = firstPoint {
                let radius = distance(center, snapped)
                guard radius > 2 / scale else { firstPoint = nil; return }
                let startAngle = atan2(snapped.y - center.y, snapped.x - center.x)
                let id = model.createArc(center: center, radius: Double(radius / scale), startAngle: Double(startAngle), endAngle: Double(startAngle + .pi))
                firstPoint = nil
                status = id != 0 ? "Дуга создана · DOF \(model.degreesOfFreedom.map(String.init) ?? "—")" : "Не удалось создать дугу"
            } else {
                firstPoint = snapped
                status = "Центр дуги выбран"
            }
        case .select:
            if let id = hitTest(snapped, in: size) {
                if constraintType != nil {
                    model.select(geometryID: id, additive: true)
                    tryApplyPendingConstraint()
                } else {
                    model.select(geometryID: id, additive: false)
                    status = "Выбрана геометрия #\(id)"
                }
            } else {
                model.clearSelection()
                status = "Точка \(Int(snapped.x)) : \(Int(snapped.y)) — пусто"
            }
        case .spline:
            pendingSpline.append(snapped)
            status = "Сплайн: точек \(pendingSpline.count) — ЛКМ добавляет, «Готово сплайн» завершает"
        case .offset:
            model.offsetSelection(distance: offsetDistance)
            status = "Смещение на \(Int(offsetDistance)) выполнено"
        default:
            status = "Инструмент «\(activeTool.title)» пока не реализован в движке"
        }
    }

    private func deleteSelected() {
        let ids = model.selectedIDs
        guard !ids.isEmpty else { return }
        for id in ids {
            model.deleteGeometry(id: id)
        }
        model.clearSelection()
        status = "Удалено геометрий: \(ids.count)"
    }

    private func finishSpline() {
        guard pendingSpline.count >= 2 else { return }
        let pts = pendingSpline
        let id = model.createSpline(points: pts)
        pendingSpline.removeAll()
        status = id != 0 ? "Сплайн создан #\(id)" : "Не удалось создать сплайн"
    }

    private func tryApplyPendingConstraint() {
        guard let type = constraintType else { return }
        let sel = model.selectedIDs
        let arr = Array(sel)
        let count = arr.count

        let requireTwo = type == .angle || type == .coincident ||
            type == .equal || type == .parallel || type == .perpendicular ||
            type == .tangent || type == .concentric || type == .symmetric
        guard count >= (requireTwo ? 2 : 1) else { return }

        let target: UInt32 = (count >= 2 && type != .radius && type != .diameter) ? arr[1] : 0
        let value = type.needsValue ? (Double(constraintValue) ?? 0) : 0
        let id = model.addConstraint(type: type, geometry: arr[0], target: target, value: value)
        if id != 0 {
            status = "Связь «\(type.title)» добавлена · DOF \(model.degreesOfFreedom.map(String.init) ?? "—")"
        } else {
            status = "Не удалось добавить связь «\(type.title)»"
        }
        model.clearSelection()
        constraintType = nil
        constraintValue = ""
    }

    private func snap(_ point: CGPoint) -> CGPoint {
        guard snapEnabled else { return point }
        if let vertex = model.snapVertex(point, tolerance: 6 / scale) {
            return vertex
        }
        let step: CGFloat = 10
        return CGPoint(x: (point.x / step).rounded() * step,
                       y: (point.y / step).rounded() * step)
    }

    private func worldToScreen(_ point: CGPoint, in size: CGSize) -> CGPoint {
        CGPoint(x: size.width / 2 + point.x * scale + pan.width,
                y: size.height / 2 - point.y * scale + pan.height)
    }

    private func screenToWorld(_ point: CGPoint, in size: CGSize) -> CGPoint {
        CGPoint(x: (point.x - size.width / 2 - pan.width) / scale,
                y: -(point.y - size.height / 2 - pan.height) / scale)
    }

    private func coordinateAxes(in size: CGSize) -> some View {
        Canvas { context, _ in
            let origin = worldToScreen(.zero, in: size)
            var xAxis = Path()
            xAxis.move(to: CGPoint(x: 0, y: origin.y))
            xAxis.addLine(to: CGPoint(x: size.width, y: origin.y))
            context.stroke(xAxis, with: .color(.red.opacity(0.5)), lineWidth: 1)

            var yAxis = Path()
            yAxis.move(to: CGPoint(x: origin.x, y: 0))
            yAxis.addLine(to: CGPoint(x: origin.x, y: size.height))
            context.stroke(yAxis, with: .color(.green.opacity(0.5)), lineWidth: 1)

            context.fill(Path(ellipseIn: CGRect(x: origin.x - 4, y: origin.y - 4, width: 8, height: 8)), with: .color(.white))
        }
        .allowsHitTesting(false)
    }

    private var constraintPanel: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("ОГРАНИЧЕНИЯ").font(.caption.bold()).foregroundStyle(.secondary)
            constraintRow("Горизонтальность", "—")
            constraintRow("Вертикальность", "—")
            constraintRow("Совпадение", "●")
            constraintRow("Параллельность", "∥")
            constraintRow("Перпендикулярность", "⊥")
            constraintRow("Касательность", "⌒")
            constraintRow("Концентричность", "◎")
            constraintRow("Равенство", "=")

            Divider().padding(.vertical, 4)
            Text("РАЗМЕРЫ").font(.caption.bold()).foregroundStyle(.secondary)
            Text("Геометрия: \(model.geometryCountValue()) · Связей: \(model.constraintCountValue())")
                .font(.caption)
                .foregroundStyle(.secondary)
            HStack(spacing: 6) {
                Circle().fill(constraintState.color).frame(width: 8, height: 8)
                Text(constraintState.text)
            }
            .font(.caption)
            .foregroundStyle(constraintState.color)
            Text(model.solverStatus)
                .font(.caption)
                .foregroundStyle(.secondary)
                .font(.caption)
                .foregroundStyle(.secondary)
            Spacer()
        }
        .frame(width: 230)
        .padding(10)
    }

    private var constraintState: (text: String, color: Color) {
        guard let dof = model.degreesOfFreedom else {
            return ("DOF: —", .secondary)
        }
        if dof == 0 { return ("Полностью зафиксировано", .green) }
        if dof < 0 { return ("Переопределено (лишние связи)", .red) }
        return ("Степени свободы: \(dof)", .orange)
    }

    private var statusBar: some View {
        HStack {
            Circle().fill(.green).frame(width: 6, height: 6)
            Text(status)
            Spacer()
            Text("Геометрия: \(model.geometryCountValue())")
            Text("Связей: \(model.constraintCountValue())")
            let cs = constraintState
            Label {
                Text(cs.text)
            } icon: {
                Circle().fill(cs.color).frame(width: 8, height: 8)
            }
            .foregroundStyle(cs.color)
            Text(model.solverStatus)
        }
        .font(.caption)
        .foregroundStyle(.secondary)
        .padding(.horizontal, 12)
        .padding(.vertical, 7)
    }

    private func toolButton(_ title: String, _ tool: SketchTool) -> some View {
        Button(title) {
            activeTool = tool
            firstPoint = nil
            status = "Инструмент: \(tool.title)"
        }
        .buttonStyle(.bordered)
        .tint(activeTool == tool ? .accentColor : nil)
    }

    private func constraintRow(_ title: String, _ symbol: String) -> some View {
        HStack {
            Text(symbol).frame(width: 22)
            Text(title)
            Spacer()
        }
        .font(.caption)
    }

    private func distance(_ a: CGPoint, _ b: CGPoint) -> CGFloat {
        hypot(b.x - a.x, b.y - a.y)
    }

    @ViewBuilder
    private func geometryView(index: UInt32, size: CGSize) -> some View {
        let id = model.geometryId(at: index)
        let isSelected = selectedIDs.contains(id)
        let stroke = isSelected ? Color.yellow : Color.cyan.opacity(0.9)
        switch model.geometryKind(at: index) {
        case .line:
            if let line = model.line(at: index) {
                Path { path in
                    path.move(to: worldToScreen(line.start, in: size))
                    path.addLine(to: worldToScreen(line.end, in: size))
                }
                .stroke(stroke, lineWidth: isSelected ? 3 : 2)
            }
        case .circle:
            if let circle = model.circle(at: index) {
                let center = worldToScreen(circle.center, in: size)
                Circle()
                    .stroke(stroke, lineWidth: isSelected ? 3 : 2)
                    .frame(width: circle.radius * 2 * scale, height: circle.radius * 2 * scale)
                    .position(center)
            }
        case .arc:
            if let arc = model.arc(at: index) {
                arcPath(center: arc.center, radius: arc.radius, start: arc.start, end: arc.end, in: size)
                    .stroke(stroke, lineWidth: isSelected ? 3 : 2)
            }
        case .spline:
            if let spl = model.spline(at: index) {
                let pts = spl.points.map { worldToScreen($0, in: size) }
                ZStack {
                    Path { path in
                        guard let first = pts.first else { return }
                        path.move(to: first)
                        for p in pts.dropFirst() { path.addLine(to: p) }
                        if spl.closed { path.closeSubpath() }
                    }
                    .stroke(stroke, lineWidth: isSelected ? 3 : 2)
                    ForEach(0..<pts.count, id: \.self) { i in
                        Rectangle()
                            .fill(isSelected ? Color.yellow : Color.white)
                            .frame(width: 5, height: 5)
                            .position(pts[i])
                    }
                }
            }
        }
    }

    @ViewBuilder
    private func previewShape(from start: CGPoint, size: CGSize) -> some View {
        let center = worldToScreen(start, in: size)
        let radius = hypot(cursor.x - center.x, cursor.y - center.y)
        switch activeTool {
        case .line:
            Path { path in
                path.move(to: center)
                path.addLine(to: cursor)
            }
            .stroke(Color.cyan.opacity(0.55), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
        case .circle:
            Circle()
                .stroke(Color.cyan.opacity(0.55), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
                .frame(width: radius * 2, height: radius * 2)
                .position(center)
        case .rectangle:
            let origin = worldToScreen(start, in: size)
            let rect = CGRect(x: min(origin.x, cursor.x), y: min(origin.y, cursor.y),
                              width: abs(cursor.x - origin.x), height: abs(cursor.y - origin.y))
            Path(rect)
                .stroke(Color.cyan.opacity(0.55), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
        default:
            EmptyView()
        }
    }

    private func hitTest(_ world: CGPoint, in size: CGSize) -> UInt32? {
        model.pickGeometry(world, tolerance: 10 / scale)
    }

    /// Опорная точка геометрии (середина линии / центр окружности) для бейджа связи.
    private func geometryAnchor(_ id: UInt32) -> CGPoint? {
        for index in 0..<model.geometryCountValue() {
            guard model.geometryId(at: index) == id else { continue }
            switch model.geometryKind(at: index) {
            case .line:
                if let l = model.line(at: index) {
                    return CGPoint(x: (l.start.x + l.end.x) / 2, y: (l.start.y + l.end.y) / 2)
                }
            case .circle, .arc:
                if let c = model.circle(at: index) { return c.center }
            case .spline:
                return nil
            }
        }
        return nil
    }

    @ViewBuilder
    private func constraintView(index: UInt32, size: CGSize) -> some View {
        if let c = model.constraint(at: index),
           let anchor = geometryAnchor(c.first) {
            let screen = worldToScreen(anchor, in: size)
            let text: String = {
                switch c.type {
                case .distance: return String(format: "%.1f", c.value)
                case .angle: return String(format: "%.0f°", c.value)
                case .radius: return String(format: "R%.1f", c.value)
                case .diameter: return String(format: "⌀%.1f", c.value)
                default: return c.type.symbol
                }
            }()
            Text(text)
                .font(.system(size: 10, weight: .semibold))
                .foregroundStyle(.orange)
                .padding(3)
                .background(.ultraThinMaterial)
                .clipShape(RoundedRectangle(cornerRadius: 4))
            .position(screen)
    }
    }

    /// Ортогональная inference: при рисовании линии привязывает угол к
    /// 0/90/180/270°, если курсор близок (в пределах ~5°).
    private func applyOrtho(_ point: CGPoint, from start: CGPoint) -> CGPoint {
        let dx = point.x - start.x
        let dy = point.y - start.y
        let dist = hypot(dx, dy)
        guard dist > 1e-6 else { return point }
        let ang = atan2(dy, dx)
        for c in [0.0, .pi / 2, .pi, -(.pi / 2), -.pi] {
            var d = ang - c
            while d > .pi { d -= 2 * .pi }
            while d < -.pi { d += 2 * .pi }
            if abs(d) < 5 * .pi / 180 {
                return CGPoint(x: start.x + dist * cos(c), y: start.y + dist * sin(c))
            }
        }
        return point
    }

    private func arcPath(center: CGPoint, radius: CGFloat, start: CGFloat, end: CGFloat, in size: CGSize) -> Path {
        let c = worldToScreen(center, in: size)
        let r = radius * scale
        var path = Path()
        path.addArc(center: c, radius: r,
                    startAngle: .radians(-Double(start)),
                    endAngle: .radians(-Double(end)),
                    clockwise: end < start)
        return path
    }
}

enum SketchTool: CaseIterable {
    case select, line, arc, circle, rectangle, trim, offset, dimension, mirror, pattern, spline

    var title: String {
        switch self {
        case .select: return "Выбор"
        case .line: return "Линия"
        case .arc: return "Дуга"
        case .circle: return "Окружность"
        case .rectangle: return "Прямоугольник"
        case .trim: return "Обрезать"
        case .offset: return "Смещение"
        case .dimension: return "Размер"
        case .mirror: return "Зеркало"
        case .pattern: return "Массив"
        case .spline: return "Сплайн"
        }
    }
}

private struct SketchGridView: View {
    let scale: CGFloat
    let pan: CGSize

    var body: some View {
        Canvas { context, size in
            let step = max(8, 20 * scale)
            var x = size.width / 2 + pan.width
            while x >= 0 { x -= step }
            while x <= size.width {
                var path = Path()
                path.move(to: CGPoint(x: x, y: 0))
                path.addLine(to: CGPoint(x: x, y: size.height))
                context.stroke(path, with: .color(.gray.opacity(0.13)), lineWidth: 1)
                x += step
            }

            var y = size.height / 2 + pan.height
            while y >= 0 { y -= step }
            while y <= size.height {
                var path = Path()
                path.move(to: CGPoint(x: 0, y: y))
                path.addLine(to: CGPoint(x: size.width, y: y))
                context.stroke(path, with: .color(.gray.opacity(0.13)), lineWidth: 1)
                y += step
            }
        }
        .allowsHitTesting(false)
    }
}

#Preview {
    SketchWorkspaceView()
        .frame(width: 1400, height: 850)
}
