import SwiftUI

struct SketchViewportView: View {
    @EnvironmentObject private var appState: CADAppState
    @StateObject private var controller = SketchInputController()
    @StateObject private var navigation = SketchViewportNavigation()

    @State private var lastMagnification: CGFloat = 1.0
    @State private var showGrid = true

    var body: some View {
        VStack(spacing: 0) {
            sketchToolbar
            Divider()
            canvas
            Divider()
            statusBar
        }
        .background(MirTheme.Colors.viewport)
        .onAppear { controller.setSubMode(appState.subMode) }
        .onChange(of: appState.subMode) { _, newMode in controller.setSubMode(newMode) }
        .onChange(of: appState.sketchPlane) { _, newPlane in
            if let plane = newPlane {
                controller.setPlane(plane)
                Mir4DSetActiveCameraPreset(plane.preset, animated: true)
            }
        }
    }

    private var sketchToolbar: some View {
        HStack(spacing: 6) {
            Label("ЭСКИЗ", systemImage: "pencil.and.ruler")
                .font(.headline)
                .padding(.horizontal, 8)

            switch controller.subMode {
            case .sketchCreate:
                toolButton("Линия", .line, systemImage: "line.diagonal")
                toolButton("Окружность", .circle, systemImage: "circle")
                toolButton("Прямоугольник", .rectangle, systemImage: "rectangle")
                toolButton("Дуга", .arc, systemImage: "arc")
                toolButton("Сплайн", .spline, systemImage: "waveform.path")
            case .sketchEdit:
                toolButton("Выбор", .select, systemImage: "arrow.up.left.and.down.right.and.arrow.up.right.and.down.left")
            case .sketchConstraint:
                toolButton("Выбор", .select, systemImage: "arrow.up.left.and.down.right.and.arrow.up.right.and.down.left")
                constraintPicker
            case .sketchDimension:
                toolButton("Выбор", .select, systemImage: "arrow.up.left.and.down.right.and.arrow.up.right.and.down.left")
            default:
                toolButton("Выбор", .select, systemImage: "arrow.up.left.and.down.right.and.arrow.up.right.and.down.left")
            }

            Spacer()

            Button { controller.undo() } label: { Image(systemName: "arrow.uturn.backward") }
                .buttonStyle(.borderless)
                .disabled(!controller.canUndo)
            Button { controller.redo() } label: { Image(systemName: "arrow.uturn.forward") }
                .buttonStyle(.borderless)
                .disabled(!controller.canRedo)
            Button("Очистить", role: .destructive) { controller.clearAll() }
                .buttonStyle(.bordered)
                .controlSize(.small)
            Toggle("Сетка", isOn: $showGrid).toggleStyle(.checkbox)
        }
        .padding(8)
    }

    private var constraintPicker: some View {
        Picker("Ограничение", selection: constraintBinding) {
            Text("Совпадение").tag(Int32(0))
            Text("Горизонтальность").tag(Int32(1))
            Text("Вертикальность").tag(Int32(2))
            Text("Параллельность").tag(Int32(3))
            Text("Перпендикулярность").tag(Int32(4))
            Text("Касание").tag(Int32(5))
            Text("Концентричность").tag(Int32(6))
            Text("Равенство").tag(Int32(7))
            Text("Дистанция").tag(Int32(9))
            Text("Угол").tag(Int32(10))
        }
        .pickerStyle(.menu)
        .controlSize(.small)
    }

    private var constraintBinding: Binding<Int32> {
        Binding(get: { controller.pendingConstraintType }, set: { controller.pendingConstraintType = $0 })
    }

    private func toolButton(_ title: String, _ tool: SketchTool, systemImage: String) -> some View {
        Button {
            controller.setTool(tool)
        } label: {
            Label(title, systemImage: systemImage)
        }
        .buttonStyle(.bordered)
        .controlSize(.small)
        .tint(controller.activeTool == tool ? .accentColor : nil)
    }

    private var canvas: some View {
        GeometryReader { proxy in
            let cs = coordinateSpace(in: proxy.size)
            ZStack {
                Color.black

                if showGrid {
                    SketchInfiniteGridView(zoom: navigation.zoom, pan: navigation.pan)
                }

                axes(in: proxy.size)
                geometryLayer(cs: cs)
                previewLayer(cs: cs)
                snapOverlay(cs: cs)
                cursorOverlay(cs: cs)
            }
            .contentShape(Rectangle())
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { controller.pointerMoved(to: cs.screenToModel($0.location)) }
                    .onEnded { controller.pointerDown(at: cs.screenToModel($0.location)) }
            )
            .simultaneousGesture(
                MagnifyGesture()
                    .onChanged { value in
                        navigation.zoom(by: value.magnification / max(lastMagnification, 1e-3), around: controller.cursor)
                        lastMagnification = value.magnification
                    }
                    .onEnded { _ in lastMagnification = 1.0 }
            )
            .onKeyPress(.escape) {
                controller.cancel()
                return .handled
            }
            .onKeyPress(.return) {
                controller.finishSpline()
                return .handled
            }
            .overlay(alignment: .topTrailing) { viewportControls }
        }
    }

    private func coordinateSpace(in size: CGSize) -> SketchCoordinateSpace {
        SketchCoordinateSpace(
            origin: CGPoint(x: size.width / 2 + navigation.pan.width, y: size.height / 2 + navigation.pan.height),
            pixelsPerUnit: 24 * navigation.zoom,
            flipY: true
        )
    }

    private func axes(in size: CGSize) -> some View {
        let center = CGPoint(x: size.width / 2 + navigation.pan.width, y: size.height / 2 + navigation.pan.height)
        return Path { path in
            path.move(to: CGPoint(x: 0, y: center.y))
            path.addLine(to: CGPoint(x: size.width, y: center.y))
            path.move(to: CGPoint(x: center.x, y: 0))
            path.addLine(to: CGPoint(x: center.x, y: size.height))
        }
        .stroke(.gray.opacity(0.5), lineWidth: 1)
        .allowsHitTesting(false)
    }

    private func geometryLayer(cs: SketchCoordinateSpace) -> some View {
        ZStack {
            ForEach(controller.entities) { entity in
                geometryShape(entity, cs: cs, selected: controller.selection.contains(entity.id))
            }
            ForEach(controller.constraints) { constraint in
                constraintBadge(constraint, cs: cs)
            }
        }
        .allowsHitTesting(false)
    }

    @ViewBuilder
    private func geometryShape(_ e: SketchInputController.Entity, cs: SketchCoordinateSpace, selected: Bool) -> some View {
        let color: Color = selected ? .accentColor : .cyan
        let width: CGFloat = selected ? 2.5 : 1.8
        switch e.kind {
        case .line:
            Path { path in
                path.move(to: cs.modelToScreen(e.start))
                path.addLine(to: cs.modelToScreen(e.end))
            }
            .stroke(color, lineWidth: width)
            if selected {
                endpointMark(e.start, cs: cs)
                endpointMark(e.end, cs: cs)
            }
            dimensionLabel(e, cs: cs)
        case .circle:
            let c = cs.modelToScreen(e.center)
            Circle()
                .stroke(color, lineWidth: width)
                .frame(width: e.radius * 2 * cs.pixelsPerUnit, height: e.radius * 2 * cs.pixelsPerUnit)
                .position(c)
            if selected { endpointMark(e.center, cs: cs) }
            dimensionLabel(e, cs: cs)
        case .arc:
            arcPath(e, cs: cs)
                .stroke(color, lineWidth: width)
            if selected {
                endpointMark(e.start, cs: cs)
                endpointMark(e.end, cs: cs)
                endpointMark(e.center, cs: cs)
            }
            dimensionLabel(e, cs: cs)
        case .spline:
            let samples = sampleSpline(e.controlPoints, closed: e.closed, segmentsPerSpan: 16)
            Path { path in
                for i in 0..<max(0, samples.count - 1) {
                    if i == 0 {
                        path.move(to: cs.modelToScreen(samples[i]))
                    }
                    path.addLine(to: cs.modelToScreen(samples[i + 1]))
                }
            }
            .stroke(color, lineWidth: width)
            if selected {
                ForEach(Array(e.controlPoints.enumerated()), id: \.offset) { _, pt in
                    endpointMark(pt, cs: cs)
                }
            }
            dimensionLabel(e, cs: cs)
        }
    }

    private func arcPath(_ e: SketchInputController.Entity, cs: SketchCoordinateSpace) -> Path {
        var path = Path()
        let span = e.endAngle - e.startAngle
        let steps = max(24, Int((abs(span) / (2 * .pi)) * 64))
        for i in 0...steps {
            let a = e.startAngle + span * CGFloat(i) / CGFloat(steps)
            let pt = CGPoint(x: e.center.x + e.radius * cos(a), y: e.center.y + e.radius * sin(a))
            let s = cs.modelToScreen(pt)
            if i == 0 { path.move(to: s) } else { path.addLine(to: s) }
        }
        return path
    }

    private func endpointMark(_ p: CGPoint, cs: SketchCoordinateSpace) -> some View {
        let s = cs.modelToScreen(p)
        return Rectangle()
            .stroke(Color.accentColor, lineWidth: 1.5)
            .frame(width: 7, height: 7)
            .position(s)
    }

    private func dimensionLabel(_ e: SketchInputController.Entity, cs: SketchCoordinateSpace) -> some View {
        let info = dimensionLabelInfo(e, cs: cs)
        return Text(info.text)
            .font(.system(size: 10, design: .monospaced))
            .foregroundStyle(.yellow)
            .padding(2)
            .background(.black.opacity(0.55))
            .position(info.at)
    }

    private func dimensionLabelInfo(_ e: SketchInputController.Entity, cs: SketchCoordinateSpace) -> (text: String, at: CGPoint) {
        switch e.kind {
        case .line:
            let len = hypot(e.end.x - e.start.x, e.end.y - e.start.y)
            return (String(format: "%.1f", len), cs.modelToScreen(CGPoint(x: (e.start.x + e.end.x) / 2, y: (e.start.y + e.end.y) / 2)))
        case .circle:
            return (String(format: "⌀ %.1f", e.radius * 2), cs.modelToScreen(e.center))
        case .arc:
            return (String(format: "R %.1f", e.radius), cs.modelToScreen(e.center))
        case .spline:
            return ("", cs.modelToScreen(e.controlPoints.first ?? .zero))
        }
    }

    private func constraintBadge(_ c: SketchInputController.Constraint, cs: SketchCoordinateSpace) -> some View {
        guard let g1 = controller.entities.first(where: { $0.id == c.g1 }) else { return AnyView(EmptyView()) }
        let at = cs.modelToScreen(g1.center == .zero ? CGPoint(x: (g1.start.x + g1.end.x) / 2, y: (g1.start.y + g1.end.y) / 2) : g1.center)
        return AnyView(
            Text(constraintSymbol(c.type))
                .font(.system(size: 12, weight: .bold))
                .foregroundStyle(.green)
                .padding(2)
                .background(.black.opacity(0.5))
                .position(at)
        )
    }

    private func constraintSymbol(_ type: Int32) -> String {
        switch type {
        case 0: return "●"
        case 1: return "—"
        case 2: return "|"
        case 3: return "∥"
        case 4: return "⊥"
        case 5: return "⌒"
        case 6: return "◎"
        case 7: return "="
        case 8: return "≅"
        case 9: return "⚡"
        case 10: return "∠"
        case 11: return "R"
        case 12: return "⌀"
        default: return "?"
        }
    }

    private func previewLayer(cs: SketchCoordinateSpace) -> some View {
        ZStack {
            if let line = controller.previewLine {
                Path { path in
                    path.move(to: cs.modelToScreen(line.0))
                    path.addLine(to: cs.modelToScreen(line.1))
                }
                .stroke(.cyan.opacity(0.6), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
            }
            if let circle = controller.previewCircle {
                let c = cs.modelToScreen(circle.0)
                Circle()
                    .stroke(.cyan.opacity(0.6), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
                    .frame(width: circle.1 * 2 * cs.pixelsPerUnit, height: circle.1 * 2 * cs.pixelsPerUnit)
                    .position(c)
            }
            if let rect = controller.previewRect {
                let a = cs.modelToScreen(rect.0)
                let b = cs.modelToScreen(rect.1)
                Path { path in
                    path.move(to: a)
                    path.addLine(to: CGPoint(x: b.x, y: a.y))
                    path.addLine(to: b)
                    path.addLine(to: CGPoint(x: a.x, y: b.y))
                    path.closeSubpath()
                }
                .stroke(.cyan.opacity(0.6), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
            }
            if let arc = controller.previewArc {
                Path { path in
                    path.move(to: cs.modelToScreen(arc.0))
                    path.addLine(to: cs.modelToScreen(arc.1))
                    path.addLine(to: cs.modelToScreen(arc.2))
                }
                .stroke(.cyan.opacity(0.6), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
            }
            if let spline = controller.previewSpline, spline.count >= 2 {
                let samples = sampleSpline(spline, closed: false, segmentsPerSpan: 16)
                Path { path in
                    for i in 0..<max(0, samples.count - 1) {
                        if i == 0 { path.move(to: cs.modelToScreen(samples[i])) }
                        path.addLine(to: cs.modelToScreen(samples[i + 1]))
                    }
                }
                .stroke(.cyan.opacity(0.6), style: StrokeStyle(lineWidth: 2, dash: [6, 4]))
            }
        }
        .allowsHitTesting(false)
    }

    private func snapOverlay(cs: SketchCoordinateSpace) -> some View {
        ZStack {
            if let p = controller.snapPoint {
                let s = cs.modelToScreen(p)
                Circle()
                    .stroke(.yellow, lineWidth: 1.5)
                    .frame(width: 12, height: 12)
                    .position(s)
            }
        }
        .allowsHitTesting(false)
    }

    private func cursorOverlay(cs: SketchCoordinateSpace) -> some View {
        let s = cs.modelToScreen(controller.cursor)
        return ZStack {
            Path { path in
                path.move(to: CGPoint(x: s.x - 10, y: s.y))
                path.addLine(to: CGPoint(x: s.x + 10, y: s.y))
                path.move(to: CGPoint(x: s.x, y: s.y - 10))
                path.addLine(to: CGPoint(x: s.x, y: s.y + 10))
            }
            .stroke(.white.opacity(0.7), lineWidth: 1)
            Text(String(format: "X %.1f  Y %.1f", controller.cursor.x, controller.cursor.y))
                .font(.system(size: 10, design: .monospaced))
                .foregroundStyle(.secondary)
                .padding(3)
                .background(.ultraThinMaterial)
                .position(x: min(max(s.x + 60, 40), 4000), y: max(s.y - 24, 12))
        }
        .allowsHitTesting(false)
    }

    private var viewportControls: some View {
        HStack(spacing: 8) {
            Text("\(Int(navigation.zoom * 100))%")
                .font(.system(size: 10, design: .monospaced))
                .foregroundStyle(.secondary)
            Button { navigation.zoom(by: 1.2, around: controller.cursor) } label: { Image(systemName: "plus.magnifyingglass") }
                .buttonStyle(.borderless)
            Button { navigation.zoom(by: 1 / 1.2, around: controller.cursor) } label: { Image(systemName: "minus.magnifyingglass") }
                .buttonStyle(.borderless)
            Button { navigation.reset() } label: { Image(systemName: "scope") }
                .buttonStyle(.borderless)
                .help("Сбросить вид (колесо — масштаб)")
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 7))
        .padding(8)
    }

    private var statusBar: some View {
        HStack {
            Circle().fill(controller.solverStatus == "Решено" ? .green : .orange)
                .frame(width: 7, height: 7)
            Text(controller.solverStatus)
            Divider().frame(height: 14)
            Text("Степени свободы: \(controller.degreesOfFreedom)")
            Text("Замкнутых профилей: \(controller.closedProfiles)")
            Text("Геометрия: \(controller.entities.count)")
            Text("Ограничения: \(controller.constraints.count)")
            Spacer()
            Text("Эскиз · \(controller.subMode.titleRU)")
        }
        .font(.caption)
        .foregroundStyle(.secondary)
        .padding(.horizontal, 12)
        .padding(.vertical, 7)
    }
}

#Preview {
    SketchViewportView()
        .environmentObject(CADAppState())
        .frame(width: 1000, height: 680)
}
