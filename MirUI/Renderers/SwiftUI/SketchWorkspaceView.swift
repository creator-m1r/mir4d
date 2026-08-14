import SwiftUI

/// Fast interactive sketch workspace.
/// Geometry remains UI-preview data until committed through the engine command bridge.
struct SketchWorkspaceView: View {
    @State private var activeTool: SketchTool = .select
    @State private var showGrid = true
    @State private var showConstraints = true
    @State private var snapEnabled = true
    @State private var status = "Эскиз готов к построению"
    @State private var points: [SketchPoint] = []
    @State private var segments: [SketchSegment] = []
    @State private var circles: [SketchCircle] = []
    @State private var firstPoint: CGPoint?
    @State private var cursor: CGPoint = .zero
    @State private var scale: CGFloat = 1.0
    @State private var pan: CGSize = .zero

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

            Spacer()

            Toggle("Сетка", isOn: $showGrid)
                .toggleStyle(.checkbox)
            Toggle("Привязки", isOn: $snapEnabled)
                .toggleStyle(.checkbox)
            Toggle("Связи", isOn: $showConstraints)
                .toggleStyle(.checkbox)

            Button("Очистить") {
                points.removeAll()
                segments.removeAll()
                circles.removeAll()
                firstPoint = nil
                status = "Эскиз очищен"
            }
            .buttonStyle(.bordered)

            Button("Завершить эскиз") {
                firstPoint = nil
                activeTool = .select
                status = "Эскиз завершён · замкнутых профилей: \(closedProfileCount)"
            }
            .buttonStyle(.borderedProminent)
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

            Divider().padding(.vertical, 4)
            Text("ОПЕРАЦИИ").font(.caption.bold()).foregroundStyle(.secondary)
            toolButton("Обрезать", .trim)
            toolButton("Смещение", .offset)
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

                ForEach(segments) { segment in
                    Path { path in
                        path.move(to: worldToScreen(segment.a, in: proxy.size))
                        path.addLine(to: worldToScreen(segment.b, in: proxy.size))
                    }
                    .stroke(Color.cyan.opacity(0.9), lineWidth: 2)
                }

                ForEach(circles) { circle in
                    let center = worldToScreen(circle.center, in: proxy.size)
                    Circle()
                        .stroke(Color.cyan.opacity(0.9), lineWidth: 2)
                        .frame(width: circle.radius * 2 * scale, height: circle.radius * 2 * scale)
                        .position(center)
                }

                ForEach(points) { point in
                    Circle()
                        .fill(point.isSnap ? Color.yellow : Color.white)
                        .frame(width: point.isSnap ? 8 : 6, height: point.isSnap ? 8 : 6)
                        .position(worldToScreen(point.position, in: proxy.size))
                }

                if activeTool == .line, let start = firstPoint {
                    Path { path in
                        path.move(to: worldToScreen(start, in: proxy.size))
                        path.addLine(to: cursor)
                    }
                    .stroke(Color.cyan.opacity(0.55), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
                }

                if activeTool == .circle, let start = firstPoint {
                    let center = worldToScreen(start, in: proxy.size)
                    let radius = hypot(cursor.x - center.x, cursor.y - center.y)
                    Circle()
                        .stroke(Color.cyan.opacity(0.55), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
                        .frame(width: radius * 2, height: radius * 2)
                        .position(center)
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
                segments.append(SketchSegment(a: start, b: snapped))
                points.append(SketchPoint(position: snapped, isSnap: true))
                firstPoint = nil
                status = "Линия создана · длина \(Int(distance(start, snapped)))"
            } else {
                firstPoint = snapped
                points.append(SketchPoint(position: snapped, isSnap: true))
                status = "Первая точка линии выбрана"
            }
        case .circle:
            if let center = firstPoint {
                let radius = distance(center, snapped)
                guard radius > 2 else { return }
                circles.append(SketchCircle(center: center, radius: radius))
                firstPoint = nil
                status = "Окружность создана · R \(Int(radius))"
            } else {
                firstPoint = snapped
                points.append(SketchPoint(position: snapped, isSnap: true))
                status = "Центр окружности выбран"
            }
        case .rectangle:
            if let start = firstPoint {
                let a = start
                let b = CGPoint(x: snapped.x, y: start.y)
                let c = snapped
                let d = CGPoint(x: start.x, y: snapped.y)
                segments.append(contentsOf: [
                    SketchSegment(a: a, b: b),
                    SketchSegment(a: b, b: c),
                    SketchSegment(a: c, b: d),
                    SketchSegment(a: d, b: a)
                ])
                points.append(contentsOf: [
                    SketchPoint(position: a, isSnap: true),
                    SketchPoint(position: b, isSnap: true),
                    SketchPoint(position: c, isSnap: true),
                    SketchPoint(position: d, isSnap: true)
                ])
                firstPoint = nil
                status = "Прямоугольник создан"
            } else {
                firstPoint = snapped
                points.append(SketchPoint(position: snapped, isSnap: true))
                status = "Первый угол выбран"
            }
        case .select:
            status = "Точка \(Int(snapped.x)) : \(Int(snapped.y)) выбрана"
        default:
            status = "Инструмент \(activeTool.title) готов к подключению к engine command bridge"
        }
    }

    private func snap(_ point: CGPoint) -> CGPoint {
        guard snapEnabled else { return point }
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
            Text("Геометрия: \(segments.count) линий · \(circles.count) окружностей")
                .font(.caption)
                .foregroundStyle(.secondary)
            Text("Замкнутых профилей: \(closedProfileCount)")
                .font(.caption)
                .foregroundStyle(.secondary)
            Spacer()
        }
        .frame(width: 230)
        .padding(10)
    }

    private var statusBar: some View {
        HStack {
            Circle().fill(.green).frame(width: 6, height: 6)
            Text(status)
            Spacer()
            Text("Геометрия: \(segments.count + circles.count)")
            Text("мм")
            Text("Solver: готов")
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

    private var closedProfileCount: Int {
        // Lightweight UI estimate; authoritative topology belongs to MirEngine.
        segments.count >= 4 && segments.count % 4 == 0 ? segments.count / 4 : 0
    }
}

enum SketchTool: CaseIterable {
    case select, line, arc, circle, rectangle, trim, offset, dimension, mirror, pattern

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
        }
    }
}

private struct SketchPoint: Identifiable {
    let id = UUID()
    let position: CGPoint
    let isSnap: Bool
}

private struct SketchSegment: Identifiable {
    let id = UUID()
    let a: CGPoint
    let b: CGPoint
}

private struct SketchCircle: Identifiable {
    let id = UUID()
    let center: CGPoint
    let radius: CGFloat
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
