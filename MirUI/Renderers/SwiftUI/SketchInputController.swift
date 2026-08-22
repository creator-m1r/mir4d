import Foundation
import CoreGraphics
import SwiftUI

/// Единая сессия режима построения эскизов.
///
/// Владеет документом MirEngine (`SketchDocument` + `SketchDocumentSolver`) и
/// зеркалирует решённую геометрию для рендера, привязки и выбора в SwiftUI.
/// Документ ядра пересобирается из проекции (`entities`/`constraints`) после
/// каждого изменения — это даёт корректные идентификаторы и стабильный undo/redo.
@MainActor
final class SketchInputController: ObservableObject {
    // MARK: - Геометрия и ограничения (проекция ядра)

    struct Entity: Identifiable, Equatable {
        let id: UInt32
        enum Kind: Int { case line = 0, arc = 1, circle = 2, spline = 3 }
        let kind: Kind
        var start: CGPoint
        var end: CGPoint
        var center: CGPoint
        var radius: CGFloat
        var startAngle: CGFloat
        var endAngle: CGFloat
        var controlPoints: [CGPoint]
        var closed: Bool

        var snapPoints: [SketchSnapCandidate] {
            switch kind {
            case .line:
                return [
                    SketchSnapCandidate(point: start, kind: .endpoint),
                    SketchSnapCandidate(point: end, kind: .endpoint),
                    SketchSnapCandidate(point: CGPoint(x: (start.x + end.x) / 2, y: (start.y + end.y) / 2), kind: .midpoint)
                ]
            case .circle, .arc:
                return [SketchSnapCandidate(point: center, kind: .center)]
            case .spline:
                guard let first = controlPoints.first, let last = controlPoints.last else { return [] }
                var pts = [SketchSnapCandidate(point: first, kind: .endpoint)]
                if controlPoints.count > 1 {
                    pts.append(SketchSnapCandidate(point: last, kind: .endpoint))
                }
                return pts
            }
        }
    }

    struct Constraint: Identifiable, Equatable {
        let id: UInt32
        let type: Int32
        let g1: UInt32
        let g2: UInt32
        let value: Double
    }

    // MARK: - Публикуемое состояние

    @Published private(set) var entities: [Entity] = []
    @Published private(set) var constraints: [Constraint] = []
    @Published private(set) var solverStatus: String = "Пусто"
    @Published private(set) var degreesOfFreedom: Int = 0
    @Published private(set) var closedProfiles: Int = 0
    @Published private(set) var selection: Set<UInt32> = []
    @Published private(set) var canUndo = false
    @Published private(set) var canRedo = false

    @Published private(set) var cursor: CGPoint = .zero
    @Published private(set) var snapPoint: CGPoint?
    @Published private(set) var snapKind: SketchSnapKind?
    @Published private(set) var previewLine: (CGPoint, CGPoint)?
    @Published private(set) var previewCircle: (CGPoint, CGFloat)?
    @Published private(set) var previewArc: (CGPoint, CGPoint, CGPoint)?
    @Published private(set) var previewRect: (CGPoint, CGPoint)?
    @Published private(set) var previewSpline: [CGPoint]?

    var activeTool: SketchTool = .line
    var subMode: CADSubMode = .sketchCreate
    var pendingConstraintType: Int32 = 0

    weak var commandBridge: SketchCommandBridge?

    /// Выбранная плоскость построения эскиза (задаётся при входе в режим).
    var planeAnchor: SketchPlaneAnchor?

    /// Провайдер проекций геометрии 3D-сцены на плоскость эскиза для привязки.
    /// Заполняется renderer'ом/сценой; по умолчанию `nil` (привязка только к
    /// собственной геометрии эскиза). Реализация позволяет «привязываться к
    /// вершинам и линиям (их проекциям)» существующей 3D-модели.
    var sceneProjectionProvider: (() -> [SketchSnapCandidate])?

    /// Анкерует документ эскиза к выбранной плоскости построения.
    func setPlane(_ anchor: SketchPlaneAnchor?) {
        planeAnchor = anchor
        guard let doc, let a = anchor else { return }
        MirEngineSketchSetPlane(
            doc, a.id,
            Float(a.origin.x), Float(a.origin.y), Float(a.origin.z),
            Float(a.normal.x), Float(a.normal.y), Float(a.normal.z),
            Float(a.xAxis.x), Float(a.xAxis.y), Float(a.xAxis.z),
            Float(a.yAxis.x), Float(a.yAxis.y), Float(a.yAxis.z))
    }

    // MARK: - Внутреннее состояние

    // nonisolated(unsafe): owned by this controller; the deinit is the only
    // nonisolated accessor, and the raw pointer is never shared across actors.
    nonisolated(unsafe) private var doc: UnsafeMutableRawPointer?
    private var nextID: UInt32 = 1
    private var past: [SketchSnapshot] = []
    private var future: [SketchSnapshot] = []

    private let snapTolerance: CGFloat = 8

    private enum Phase {
        case idle
        case lineStart(CGPoint)
        case circleCenter(CGPoint)
        case rectFirst(CGPoint)
        case arcStart(CGPoint)
        case arcEnd(CGPoint, CGPoint)
        case splineCollecting([CGPoint])
    }

    private var phase: Phase = .idle

    private enum DragState {
        case ready(id: UInt32, endpoint: Bool, ref: EndpointRef, origin: CGPoint)
        case dragging(id: UInt32, endpoint: Bool, ref: EndpointRef, last: CGPoint)
    }

    private enum EndpointRef { case start, end, whole }

    private var dragState: DragState?

    // MARK: - Жизненный цикл

    init() {
        doc = MirEngineSketchCreateDocument()
    }

    deinit {
        if let doc { MirEngineSketchDestroyDocument(doc) }
    }

    // MARK: - Подрежимы и инструменты

    func setSubMode(_ mode: CADSubMode) {
        subMode = mode
        phase = .idle
        dragState = nil
        previewLine = nil
        previewCircle = nil
        previewArc = nil
        previewRect = nil
        previewSpline = nil
        switch mode {
        case .sketchCreate: activeTool = .line
        case .sketchEdit: activeTool = .select
        case .sketchConstraint, .sketchDimension: activeTool = .select
        default: activeTool = .select
        }
    }

    func setTool(_ tool: SketchTool) {
        activeTool = tool
        phase = .idle
    }

    // MARK: - Ввод

    func pointerMoved(to world: CGPoint) {
        cursor = world

        if case let .dragging(id, endpoint, ref, _) = dragState {
            let p = snapped(world)
            applyDrag(id: id, endpoint: endpoint, ref: ref, to: p)
            dragState = .dragging(id: id, endpoint: endpoint, ref: ref, last: p)
            return
        }

        if case let .ready(id, endpoint, ref, origin) = dragState {
            let p = snapped(world)
            if hypot(p.x - origin.x, p.y - origin.y) > 1e-3 {
                startDrag()
                dragState = .dragging(id: id, endpoint: endpoint, ref: ref, last: p)
                return
            }
        }

        updatePreview(for: snapped(world))
    }

    func pointerDown(at world: CGPoint) {
        let p = snapped(world)

        if case let .dragging(id, endpoint, ref, _) = dragState {
            applyDrag(id: id, endpoint: endpoint, ref: ref, to: p)
            endDrag()
            dragState = nil
            return
        }

        dragState = nil

        switch effectiveMode {
        case .constraint:
            handleConstraintPick(p)
        case .dimension:
            handleDimensionPick(p)
        case .create, .edit:
            handleCreateOrSelect(p)
        }
    }

    func cancel() {
        phase = .idle
        dragState = nil
        previewLine = nil
        previewCircle = nil
        previewArc = nil
        previewRect = nil
        previewSpline = nil
    }

    // MARK: - Реализация ввода

    private var effectiveMode: Mode {
        switch subMode {
        case .sketchConstraint: return .constraint
        case .sketchDimension: return .dimension
        default: return activeTool == .select ? .edit : .create
        }
    }

    private enum Mode { case create, edit, constraint, dimension }

    private func handleCreateOrSelect(_ p: CGPoint) {
        switch activeTool {
        case .line:
            if case let .lineStart(start) = phase {
                commitLine(start, p)
                phase = .idle
            } else {
                phase = .lineStart(p)
            }
        case .circle:
            if case let .circleCenter(c) = phase {
                let r = max(hypot(p.x - c.x, p.y - c.y), 0.5)
                commitCircle(c, r)
                phase = .idle
            } else {
                phase = .circleCenter(p)
            }
        case .rectangle:
            if case let .rectFirst(corner) = phase {
                commitRectangle(corner, p)
                phase = .idle
            } else {
                phase = .rectFirst(p)
            }
        case .arc:
            switch phase {
            case let .arcStart(s):
                phase = .arcEnd(s, p)
            case let .arcEnd(s, e):
                commitArc(s, e, p)
                phase = .idle
            default:
                phase = .arcStart(p)
            }
        case .spline:
            if case let .splineCollecting(pts) = phase {
                if pts.count >= 2, let first = pts.first,
                   hypot(p.x - first.x, p.y - first.y) <= snapTolerance {
                    commitSpline(pts, closed: true)
                    phase = .idle
                } else if let last = pts.last,
                          hypot(p.x - last.x, p.y - last.y) <= snapTolerance {
                    // ignore a click on top of the previous point
                } else {
                    phase = .splineCollecting(pts + [p])
                }
            } else {
                phase = .splineCollecting([p])
            }
        case .select, .trim:
            if dragState == nil {
                selectAt(p)
            }
        default:
            break
        }
    }

    private func selectAt(_ p: CGPoint) {
        guard let hit = hitTest(p) else {
            selection.removeAll()
            dragState = nil
            return
        }
        selection = [hit.id]
        dragState = .ready(id: hit.id, endpoint: hit.endpoint, ref: hit.ref, origin: p)
    }

    private func handleConstraintPick(_ p: CGPoint) {
        guard let hit = hitTest(p) else { return }
        if selection.isEmpty {
            selection = [hit.id]
        } else if let first = selection.first, first != hit.id {
            let g1 = first
            let g2 = hit.id
            let type = pendingConstraintType
            commit {
                self.constraints.append(Constraint(id: UInt32(self.constraints.count + 1), type: type, g1: g1, g2: g2, value: 0))
            }
            selection = [g2]
        } else {
            selection = [hit.id]
        }
    }

    private func handleDimensionPick(_ p: CGPoint) {
        guard let hit = hitTest(p) else { return }
        guard let e = entities.first(where: { $0.id == hit.id }) else { return }
        switch e.kind {
        case .line:
            let len = hypot(e.end.x - e.start.x, e.end.y - e.start.y)
            commit { self.constraints.append(Constraint(id: UInt32(self.constraints.count + 1), type: 10, g1: e.id, g2: 0, value: Double(len))) }
        case .circle, .arc:
            commit { self.constraints.append(Constraint(id: UInt32(self.constraints.count + 1), type: 11, g1: e.id, g2: 0, value: Double(e.radius))) }
        case .spline:
            let samples = sampleSpline(e.controlPoints, closed: e.closed, segmentsPerSpan: 16)
            var length: Double = 0
            for i in 0..<max(0, samples.count - 1) {
                length += Double(hypot(samples[i + 1].x - samples[i].x, samples[i + 1].y - samples[i].y))
            }
            commit { self.constraints.append(Constraint(id: UInt32(self.constraints.count + 1), type: 10, g1: e.id, g2: 0, value: length)) }
        }
        selection = [e.id]
    }

    // MARK: - Коммиты геометрии

    @discardableResult
    func commitLine(_ start: CGPoint, _ end: CGPoint) -> UInt32 {
        let id = nextID
        nextID += 1
        let e = Entity(id: id, kind: .line, start: start, end: end, center: .zero, radius: 0, startAngle: 0, endAngle: 0, controlPoints: [], closed: false)
        commit { self.entities.append(e) }
        return id
    }

    @discardableResult
    func commitCircle(_ center: CGPoint, _ radius: CGFloat) -> UInt32 {
        let id = nextID
        nextID += 1
        let e = Entity(id: id, kind: .circle, start: .zero, end: .zero, center: center, radius: radius, startAngle: 0, endAngle: 0, controlPoints: [], closed: false)
        commit { self.entities.append(e) }
        return id
    }

    @discardableResult
    func commitRectangle(_ a: CGPoint, _ b: CGPoint) -> [UInt32] {
        let c = CGPoint(x: b.x, y: a.y)
        let d = CGPoint(x: a.x, y: b.y)
        let ids = [
            commitLine(a, c),
            commitLine(c, b),
            commitLine(b, d),
            commitLine(d, a)
        ]
        return ids
    }

    @discardableResult
    func commitSpline(_ points: [CGPoint], closed: Bool) -> UInt32 {
        let id = nextID
        nextID += 1
        let e = Entity(id: id, kind: .spline, start: .zero, end: .zero, center: .zero,
                       radius: 0, startAngle: 0, endAngle: 0, controlPoints: points, closed: closed)
        commit { self.entities.append(e) }
        return id
    }

    /// Завершает набор контрольных точек сплайна (Enter) как разомкнутую кривую.
    func finishSpline() {
        if case let .splineCollecting(pts) = phase, pts.count >= 2 {
            commitSpline(pts, closed: false)
        }
        phase = .idle
    }

    @discardableResult
    func commitArc(_ s: CGPoint, _ e: CGPoint, _ bulge: CGPoint) -> UInt32 {
        guard let arc = arcFromPoints(s, e, bulge) else {
            return commitLine(s, e)
        }
        let id = nextID
        nextID += 1
        let ent = Entity(id: id, kind: .arc, start: s, end: e, center: arc.center, radius: arc.radius, startAngle: arc.startAngle, endAngle: arc.endAngle, controlPoints: [], closed: false)
        commit { self.entities.append(ent) }
        return id
    }

    func deleteSelected() {
        let sel = selection
        guard !sel.isEmpty else { return }
        commit {
            self.entities.removeAll { sel.contains($0.id) }
            self.constraints.removeAll { sel.contains($0.g1) || sel.contains($0.g2) }
        }
        selection.removeAll()
    }

    func clearAll() {
        guard !entities.isEmpty || !constraints.isEmpty else { return }
        commit {
            self.entities.removeAll()
            self.constraints.removeAll()
        }
        selection.removeAll()
    }

    // MARK: - Drag

    private func startDrag() {
        past.append(SketchSnapshot(entities: entities, constraints: constraints))
        future.removeAll()
        updateHistoryFlags()
    }

    private func endDrag() {
        updateHistoryFlags()
    }

    private func applyDrag(id: UInt32, endpoint: Bool, ref: EndpointRef, to p: CGPoint) {
        guard var e = entities.first(where: { $0.id == id }) else { return }
        if e.kind == .line {
            switch ref {
            case .start: e.start = p
            case .end: e.end = p
            case .whole: break
            }
        } else {
            e.center = p
        }
        liveUpdate(e)
    }

    private func liveUpdate(_ e: Entity) {
        if let idx = entities.firstIndex(where: { $0.id == e.id }) {
            entities[idx] = e
        }
        rebuildEngine()
    }

    // MARK: - Undo / Redo

    func undo() {
        guard let last = past.popLast() else { return }
        future.append(SketchSnapshot(entities: entities, constraints: constraints))
        restore(last)
        updateHistoryFlags()
    }

    func redo() {
        guard let next = future.popLast() else { return }
        past.append(SketchSnapshot(entities: entities, constraints: constraints))
        restore(next)
        updateHistoryFlags()
    }

    private func updateHistoryFlags() {
        canUndo = !past.isEmpty
        canRedo = !future.isEmpty
    }

    private func commit(_ mutate: () -> Void) {
        past.append(SketchSnapshot(entities: entities, constraints: constraints))
        future.removeAll()
        mutate()
        rebuildEngine()
        updateHistoryFlags()
    }

    private func restore(_ snap: SketchSnapshot) {
        entities = snap.entities
        constraints = snap.constraints
        rebuildEngine()
    }

    // MARK: - Пересборка ядра

    private func rebuildEngine() {
        guard let doc else { return }
        MirEngineSketchClear(doc)

        var map: [UInt32: UInt32] = [:]
        for e in entities {
            let engineID: UInt32
            switch e.kind {
            case .line:
                engineID = MirEngineSketchAddLine(doc, Float(e.start.x), Float(e.start.y), Float(e.end.x), Float(e.end.y))
            case .circle:
                engineID = MirEngineSketchAddCircle(doc, Float(e.center.x), Float(e.center.y), Float(e.radius))
            case .arc:
                engineID = MirEngineSketchAddArc(doc, Float(e.center.x), Float(e.center.y), Float(e.radius), Float(e.startAngle), Float(e.endAngle))
            case .spline:
                let xs = e.controlPoints.map { Float($0.x) }
                let ys = e.controlPoints.map { Float($0.y) }
                engineID = MirEngineSketchAddSpline(doc, xs, ys, UInt32(xs.count), e.closed)
            }
            map[e.id] = engineID
        }

        for c in constraints {
            let g1 = map[c.g1] ?? c.g1
            let g2 = map[c.g2] ?? c.g2
            _ = MirEngineSketchAddConstraint(doc, Int32(c.type), g1, g2, c.value)
        }

        let ok = MirEngineSketchSolve(doc)
        solverStatus = entities.isEmpty ? "Пусто" : (ok ? "Решено" : "Не сходится")
        syncBack()
        closedProfiles = detectClosedProfiles()
        degreesOfFreedom = computeDegreesOfFreedom()
        pushToRenderer()
    }

    /// Mirrors the solved 2D sketch onto the active 3D work plane so the geometry
    /// is drawn on the plane inside the 3D viewport (not only in the 2D overlay).
    /// The 2D canvas model space (origin at screen centre, x→right, y→up) aligns
    /// with the plane's (u, v) parameterisation once the camera is oriented to
    /// the plane preset, so the same coordinates land on the plane in world space.
    private func pushToRenderer() {
        guard let renderer = MIR4DModelRuntime.shared.renderer else { return }
        let strokeColor: (Float, Float, Float) = (0.2, 0.8, 1.0)

        guard let plane = planeAnchor, !entities.isEmpty else {
            MirEnginePushSketch(renderer, [])
            return
        }

        var segs: [MirEngineSketchSegment] = []
        segs.reserveCapacity(entities.count * 2)
        for e in entities {
            switch e.kind {
            case .line:
                segs.append(MirEngineSketchSegment(
                    ax: Float(e.start.x), ay: Float(e.start.y),
                    bx: Float(e.end.x), by: Float(e.end.y), color: strokeColor))
            case .circle:
                let n = 48
                for i in 0..<n {
                    let a0 = 2 * Double.pi * Double(i) / Double(n)
                    let a1 = 2 * Double.pi * Double(i + 1) / Double(n)
                    segs.append(MirEngineSketchSegment(
                        ax: Float(e.center.x + e.radius * cos(a0)),
                        ay: Float(e.center.y + e.radius * sin(a0)),
                        bx: Float(e.center.x + e.radius * cos(a1)),
                        by: Float(e.center.y + e.radius * sin(a1)),
                        color: strokeColor))
                }
            case .arc:
                let span = e.endAngle - e.startAngle
                let n = max(2, Int((abs(span) / (2 * Double.pi)) * 64))
                for i in 0..<n {
                    let a0 = e.startAngle + span * Double(i) / Double(n)
                    let a1 = e.startAngle + span * Double(i + 1) / Double(n)
                    segs.append(MirEngineSketchSegment(
                        ax: Float(e.center.x + e.radius * cos(a0)),
                        ay: Float(e.center.y + e.radius * sin(a0)),
                        bx: Float(e.center.x + e.radius * cos(a1)),
                        by: Float(e.center.y + e.radius * sin(a1)),
                        color: strokeColor))
                }
            case .spline:
                let samples = sampleSpline(e.controlPoints, closed: e.closed, segmentsPerSpan: 16)
                for i in 0..<max(0, samples.count - 1) {
                    segs.append(MirEngineSketchSegment(
                        ax: Float(samples[i].x), ay: Float(samples[i].y),
                        bx: Float(samples[i + 1].x), by: Float(samples[i + 1].y),
                        color: strokeColor))
                }
            }
        }

        let origin: [Float] = [Float(plane.origin.x), Float(plane.origin.y), Float(plane.origin.z)]
        let xAxis: [Float] = [Float(plane.xAxis.x), Float(plane.xAxis.y), Float(plane.xAxis.z)]
        let yAxis: [Float] = [Float(plane.yAxis.x), Float(plane.yAxis.y), Float(plane.yAxis.z)]
        MirEnginePushSketch(renderer, segs, origin: origin, xAxis: xAxis, yAxis: yAxis)
    }

    /// Выдавливает нарисованный эскиз в твёрдое тело на активной плоскости.
    /// Профиль — габаритный прямоугольник эскиза в локальных координатах
    /// плоскости (U,V); глубина — `distance`. Возвращает id созданного объекта.
    // MARK: - Экструзия (выдавливание профиля)

    /// id последнего выдавленного тела этого сеанса эскиза (для редактирования).
    var lastExtrudedId: UInt64?
    /// Краткая сводка метрик последнего тела (JSON-строка от ядра).
    @Published var lastMetricsJson: String = ""
    /// Глубина, с которой было создано последнее тело (база для масштаба).
    private var lastExtrudeDepth: Double = 1.0
    /// Базовый transform последнего тела (позиция/поворот; масштаб = 1).
    private var lastBaseTransform: MirTransform?
    /// Базовая позиция и оси плоскости для параметрического перемещения.
    private var moveBasePos: (Double, Double, Double)?
    private var moveAxisU: (Double, Double, Double)?
    private var moveAxisV: (Double, Double, Double)?
    private var moveAxisN: (Double, Double, Double)?

    @discardableResult
    func extrude(distance: Double) -> UInt64? {
        guard let plane = planeAnchor,
              let renderer = MIR4DModelRuntime.shared.renderer else { return nil }

        // Редактирование: при повторной экструзии заменяем прежнее тело.
        if let prev = lastExtrudedId {
            mirEngineSelectObject(renderer, prev)
            _ = MirEngineDeleteSelectedObject(renderer)
            lastExtrudedId = nil
        }

        guard !entities.isEmpty else { return nil }

        let id: UInt64?
        if closedProfiles >= 1, let contour = orderedClosedContour(), contour.count >= 3 {
            var flat: [Double] = []
            flat.reserveCapacity(contour.count * 2)
            for p in contour { flat.append(p.x); flat.append(p.y) }
            id = flat.withUnsafeBufferPointer { buf in
                MirEngineExtrudeContour(
                    renderer, buf.baseAddress, Int32(contour.count), distance,
                    plane.origin.x, plane.origin.y, plane.origin.z,
                    plane.normal.x, plane.normal.y, plane.normal.z,
                    plane.xAxis.x, plane.xAxis.y, plane.xAxis.z,
                    plane.yAxis.x, plane.yAxis.y, plane.yAxis.z)
            }
        } else {
            // Запасной вариант: габаритный прямоугольник наброска.
            var uMin = Double.infinity, uMax = -Double.infinity
            var vMin = Double.infinity, vMax = -Double.infinity
            func include(_ p: CGPoint) {
                uMin = min(uMin, p.x); uMax = max(uMax, p.x)
                vMin = min(vMin, p.y); vMax = max(vMax, p.y)
            }
            for e in entities {
                switch e.kind {
                case .line:
                    include(e.start); include(e.end)
                case .circle:
                    include(CGPoint(x: e.center.x - e.radius, y: e.center.y - e.radius))
                    include(CGPoint(x: e.center.x + e.radius, y: e.center.y + e.radius))
                case .arc:
                    let steps = max(8, Int((abs(e.endAngle - e.startAngle) / (2 * .pi)) * 32))
                    for i in 0...steps {
                        let a = e.startAngle + (e.endAngle - e.startAngle) * Double(i) / Double(steps)
                        include(CGPoint(x: e.center.x + e.radius * cos(a), y: e.center.y + e.radius * sin(a)))
                    }
                case .spline:
                    for s in sampleSpline(e.controlPoints, closed: e.closed, segmentsPerSpan: 8) { include(s) }
                }
            }
            guard uMin < uMax, vMin < vMax else { return nil }
            let idBox = MirEngineExtrudeSketch(
                renderer, uMax - uMin, vMax - vMin, (uMin + uMax) / 2, (vMin + vMax) / 2, distance,
                plane.origin.x, plane.origin.y, plane.origin.z,
                plane.normal.x, plane.normal.y, plane.normal.z,
                plane.xAxis.x, plane.xAxis.y, plane.xAxis.z,
                plane.yAxis.x, plane.yAxis.y, plane.yAxis.z)
            id = idBox != 0 ? idBox : nil
        }

        guard let created = id, created != 0 else { return nil }
        lastExtrudedId = created
        mirEngineSelectObject(renderer, created)
        var buf = [CChar](repeating: 0, count: 1024)
        if MirEngineGetObjectMetricsById(renderer, created, &buf, buf.count) {
            lastMetricsJson = String(cString: buf)
        } else {
            lastMetricsJson = ""
        }
        // Запоминаем базовый transform, глубину и базис плоскости для правки.
        var t = MirTransform()
        if MirEngineGetObjectTransform(renderer, created, &t) {
            lastBaseTransform = t
            lastExtrudeDepth = distance
            moveBasePos = (t.px, t.py, t.pz)
            if let plane = planeAnchor {
                moveAxisU = (plane.xAxis.x, plane.xAxis.y, plane.xAxis.z)
                moveAxisV = (plane.yAxis.x, plane.yAxis.y, plane.yAxis.z)
                moveAxisN = (plane.normal.x, plane.normal.y, plane.normal.z)
            }
        }
        return created
    }

    /// Параметрически меняет глубину последнего тела БЕЗ пересоздания геометрии:
    /// масштабируем узел вдоль локальной Z (нормали плоскости). Основание
    /// остаётся на плоскости, высота = depth.
    func updateDepth(_ depth: Double) {
        guard let id = lastExtrudedId, let base = lastBaseTransform,
              let renderer = MIR4DModelRuntime.shared.renderer,
              lastExtrudeDepth > 0 else { return }
        var t = base
        t.sx = 1; t.sy = 1
        t.sz = depth / lastExtrudeDepth
        MirEngineSetObjectTransform(renderer, id, &t)
    }

    /// Параметрически перемещает последнее тело БЕЗ пересоздания геометрии.
    /// (u, v) — сдвиг вдоль осей плоскости, n — вдоль нормали. Поворот и
    /// масштаб (глубина) сохраняются.
    func updateMove(_ u: Double, _ v: Double, _ n: Double) {
        guard let id = lastExtrudedId,
              let bp = moveBasePos, let au = moveAxisU,
              let av = moveAxisV, let an = moveAxisN,
              let renderer = MIR4DModelRuntime.shared.renderer else { return }
        var t = MirTransform()
        guard MirEngineGetObjectTransform(renderer, id, &t) else { return }
        t.px = bp.0 + au.0 * u + av.0 * v + an.0 * n
        t.py = bp.1 + au.1 * u + av.1 * v + an.1 * n
        t.pz = bp.2 + au.2 * u + av.2 * v + an.2 * n
        MirEngineSetObjectTransform(renderer, id, &t)
    }

    // MARK: - Извлечение замкнутого контура

    /// Возвращает упорядоченный замкнутый контур эскиза (в локальных UV),
    /// годный для выдавливания, либо nil.
    private func orderedClosedContour() -> [CGPoint]? {
        guard closedProfiles >= 1 else { return nil }

        var edges: [[CGPoint]] = []
        for e in entities {
            switch e.kind {
            case .line:
                edges.append([e.start, e.end])
            case .arc:
                edges.append(sampledArc(e))
            case .circle:
                edges.append(sampledCircle(e))
            case .spline:
                let s = sampleSpline(e.controlPoints, closed: e.closed, segmentsPerSpan: 12)
                if s.count >= 2 { edges.append(s) }
            }
        }
        guard edges.count >= 3 else { return nil }

        let tol: CGFloat = 1e-3
        func close(_ a: CGPoint, _ b: CGPoint) -> Bool { hypot(a.x - b.x, a.y - b.y) < tol }

        var remaining = edges
        var loop: [CGPoint] = []
        var current = remaining.removeFirst()
        loop.append(contentsOf: current)
        var safety = 0
        while safety < 5000 {
            safety += 1
            let tail = loop.last!
            if close(loop.first!, tail) { break }
            if let idx = remaining.firstIndex(where: { close($0.first!, tail) }) {
                let next = remaining.remove(at: idx)
                loop.append(contentsOf: next.dropFirst())
            } else if let idx = remaining.firstIndex(where: { close($0.last!, tail) }) {
                var next = remaining.remove(at: idx)
                next.reverse()
                loop.append(contentsOf: next.dropFirst())
            } else {
                break
            }
        }

        // Убираем дубликаты подряд идущих точек (вырожденные рёбра).
        var cleaned: [CGPoint] = []
        for p in loop {
            if cleaned.last.map({ close($0, p) }) != true { cleaned.append(p) }
        }
        loop = cleaned

        let isClosed = close(loop.first ?? .zero, loop.last ?? .zero) ||
            (remaining.isEmpty && edges.count == 1 && loop.count >= 4)
        guard isClosed, loop.count >= 4 else { return nil }
        if let first = loop.first, let last = loop.last, close(first, last) { loop.removeLast() }
        return loop
    }

    private func sampledArc(_ e: Entity) -> [CGPoint] {
        let sweep = abs(e.endAngle - e.startAngle)
        let steps = max(8, Int((sweep / (2 * .pi)) * 48))
        var pts: [CGPoint] = []
        pts.reserveCapacity(steps + 1)
        for i in 0...steps {
            let a = e.startAngle + (e.endAngle - e.startAngle) * Double(i) / Double(steps)
            pts.append(CGPoint(x: e.center.x + e.radius * cos(a), y: e.center.y + e.radius * sin(a)))
        }
        return pts
    }

    private func sampledCircle(_ e: Entity) -> [CGPoint] {
        let steps = 48
        var pts: [CGPoint] = []
        pts.reserveCapacity(steps)
        for i in 0..<steps {
            let a = 2 * .pi * Double(i) / Double(steps)
            pts.append(CGPoint(x: e.center.x + e.radius * cos(a), y: e.center.y + e.radius * sin(a)))
        }
        return pts
    }

    private func syncBack() {
        guard let doc else { return }
        let count = MirEngineSketchGeometryCount(doc)
        var synced: [Entity] = []
        for i in 0..<count {
            var e = (Int(i) < entities.count) ? entities[Int(i)] : Entity(id: 0, kind: .line, start: .zero, end: .zero, center: .zero, radius: 0, startAngle: 0, endAngle: 0, controlPoints: [], closed: false)
            readEntity(&e, at: UInt32(i))
            synced.append(e)
        }
        entities = synced

        var newConstraints: [Constraint] = []
        let cc = MirEngineSketchConstraintCount(doc)
        for i in 0..<cc {
            var t: Int32 = 0, g1: UInt32 = 0, g2: UInt32 = 0
            var v: Double = 0
            if MirEngineSketchConstraintAt(doc, UInt32(i), &t, &g1, &g2, &v) {
                newConstraints.append(Constraint(id: UInt32(i) + 1, type: t, g1: g1, g2: g2, value: v))
            }
        }
        constraints = newConstraints
    }

    private func readEntity(_ e: inout Entity, at index: UInt32) {
        guard let doc else { return }
        let kind = MirEngineSketchGeometryTypeAt(doc, index)
        switch kind {
        case 0:
            var x1: Float = 0, y1: Float = 0, x2: Float = 0, y2: Float = 0
            if MirEngineSketchLineAt(doc, index, &x1, &y1, &x2, &y2) {
                e.start = CGPoint(x: CGFloat(x1), y: CGFloat(y1))
                e.end = CGPoint(x: CGFloat(x2), y: CGFloat(y2))
            }
        case 1:
            var cx: Float = 0, cy: Float = 0, r: Float = 0, sa: Float = 0, ea: Float = 0
            if MirEngineSketchArcAt(doc, index, &cx, &cy, &r, &sa, &ea) {
                e.center = CGPoint(x: CGFloat(cx), y: CGFloat(cy))
                e.radius = CGFloat(r)
                e.startAngle = CGFloat(sa)
                e.endAngle = CGFloat(ea)
            }
        case 2:
            var cx: Float = 0, cy: Float = 0, r: Float = 0
            if MirEngineSketchCircleAt(doc, index, &cx, &cy, &r) {
                e.center = CGPoint(x: CGFloat(cx), y: CGFloat(cy))
                e.radius = CGFloat(r)
            }
        case 3:
            var cnt: UInt32 = UInt32(e.controlPoints.count)
            var closed: Bool = false
            let n = Int(cnt)
            var xs = [Float](repeating: 0, count: max(n, 1))
            var ys = [Float](repeating: 0, count: max(n, 1))
            if MirEngineSketchSplineAt(doc, index, &xs, &ys, &cnt, &closed) {
                var pts: [CGPoint] = []
                pts.reserveCapacity(Int(cnt))
                for i in 0..<Int(cnt) {
                    pts.append(CGPoint(x: CGFloat(xs[i]), y: CGFloat(ys[i])))
                }
                e.controlPoints = pts
                e.closed = closed
            }
        default:
            break
        }
    }

    // MARK: - Привязка

    func snapped(_ world: CGPoint) -> CGPoint {
        var best: (point: CGPoint, kind: SketchSnapKind, dist: CGFloat)?
        for e in entities {
            for cand in e.snapPoints {
                let d = hypot(cand.point.x - world.x, cand.point.y - world.y)
                if d <= snapTolerance {
                    if best == nil || d < best!.dist {
                        best = (cand.point, cand.kind, d)
                    }
                }
            }
        }
        if let provider = sceneProjectionProvider {
            for cand in provider() {
                let d = hypot(cand.point.x - world.x, cand.point.y - world.y)
                if d <= snapTolerance {
                    if best == nil || d < best!.dist {
                        best = (cand.point, cand.kind, d)
                    }
                }
            }
        }

        if let best {
            snapPoint = best.point
            snapKind = best.kind
            return best.point
        }
        snapPoint = nil
        snapKind = nil
        return world
    }

    // MARK: - Hit-тест

    private func hitTest(_ p: CGPoint) -> (id: UInt32, endpoint: Bool, ref: EndpointRef)? {
        var best: (id: UInt32, endpoint: Bool, ref: EndpointRef, dist: CGFloat)?
        for e in entities {
            switch e.kind {
            case .line:
                let ds = hypot(e.start.x - p.x, e.start.y - p.y)
                let de = hypot(e.end.x - p.x, e.end.y - p.y)
                if ds <= snapTolerance, best == nil || ds < best!.dist { best = (e.id, true, .start, ds) }
                if de <= snapTolerance, best == nil || de < best!.dist { best = (e.id, true, .end, de) }
                let d = distanceToSegment(p, e.start, e.end)
                if d <= snapTolerance, best == nil || d < best!.dist { best = (e.id, false, .whole, d) }
            case .circle, .arc:
                let d = abs(hypot(e.center.x - p.x, e.center.y - p.y) - e.radius)
                if d <= snapTolerance, best == nil || d < best!.dist { best = (e.id, false, .whole, d) }
            case .spline:
                let samples = sampleSpline(e.controlPoints, closed: e.closed, segmentsPerSpan: 16)
                for i in 0..<max(0, samples.count - 1) {
                    let d = distanceToSegment(p, samples[i], samples[i + 1])
                    if d <= snapTolerance, best == nil || d < best!.dist { best = (e.id, false, .whole, d) }
                }
            }
        }
        if let best {
            return (best.id, best.endpoint, best.ref)
        }
        return nil
    }

    // MARK: - Превью

    private func updatePreview(for p: CGPoint) {
        previewLine = nil
        previewCircle = nil
        previewArc = nil
        previewRect = nil
        switch activeTool {
        case .line:
            if case let .lineStart(s) = phase { previewLine = (s, p) }
        case .circle:
            if case let .circleCenter(c) = phase { previewCircle = (c, max(hypot(p.x - c.x, p.y - c.y), 0.5)) }
        case .rectangle:
            if case let .rectFirst(c) = phase { previewRect = (c, p) }
        case .arc:
            switch phase {
            case let .arcStart(s): previewLine = (s, p)
            case let .arcEnd(s, e): previewArc = (s, e, p)
            default: break
            }
        case .spline:
            if case let .splineCollecting(pts) = phase {
                previewSpline = pts + [p]
            }
        default:
            break
        }
    }

    // MARK: - Аналитика

    private func distanceToSegment(_ p: CGPoint, _ a: CGPoint, _ b: CGPoint) -> CGFloat {
        let dx = b.x - a.x
        let dy = b.y - a.y
        let len2 = dx * dx + dy * dy
        guard len2 > 0 else { return hypot(p.x - a.x, p.y - a.y) }
        var t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / len2
        t = min(max(t, 0), 1)
        return hypot(p.x - (a.x + t * dx), p.y - (a.y + t * dy))
    }

    private func detectClosedProfiles() -> Int {
        let lines = entities.filter { $0.kind == .line }
        guard lines.count >= 3 else { return 0 }
        func key(_ p: CGPoint) -> String { "\(Int(round(p.x * 1000))):\(Int(round(p.y * 1000)))" }
        var degree: [String: Int] = [:]
        for l in lines {
            degree[key(l.start), default: 0] += 1
            degree[key(l.end), default: 0] += 1
        }
        return degree.values.allSatisfy { $0 == 2 } ? 1 : 0
    }

    private func computeDegreesOfFreedom() -> Int {
        var points = Set<String>()
        func key(_ p: CGPoint) -> String { "\(Int(round(p.x * 1000))):\(Int(round(p.y * 1000)))" }
        for e in entities {
            switch e.kind {
            case .line:
                points.insert(key(e.start))
                points.insert(key(e.end))
            case .circle, .arc:
                points.insert(key(e.center))
            case .spline:
                for p in e.controlPoints { points.insert(key(p)) }
            }
        }
        return max(0, 2 * points.count - constraints.count)
    }

    /// Samples a Catmull-Rom spline through `controlPoints`.
    private func sampleSpline(_ controlPoints: [CGPoint], closed: Bool, segmentsPerSpan: Int) -> [CGPoint] {
        let n = controlPoints.count
        guard n >= 2 else { return controlPoints }
        var result: [CGPoint] = []
        func catmull(_ p0: CGPoint, _ p1: CGPoint, _ p2: CGPoint, _ p3: CGPoint, _ t: CGFloat) -> CGPoint {
            let t2 = t * t
            let t3 = t2 * t
            let a0: CGFloat = 2 * p1.x
            let a1: CGFloat = (-p0.x + p2.x) * t
            let a2: CGFloat = (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2
            let a3: CGFloat = (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3
            let b0: CGFloat = 2 * p1.y
            let b1: CGFloat = (-p0.y + p2.y) * t
            let b2: CGFloat = (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2
            let b3: CGFloat = (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3
            return CGPoint(x: 0.5 * (a0 + a1 + a2 + a3), y: 0.5 * (b0 + b1 + b2 + b3))
        }
        let spans = n - (closed ? 0 : 1)
        for i in 0..<spans {
            let p0 = controlPoints[closed ? (i + n - 1) % n : max(0, i - 1)]
            let p1 = controlPoints[i % n]
            let p2 = controlPoints[(i + 1) % n]
            let p3 = controlPoints[(i + 2) % n]
            for s in 0..<segmentsPerSpan {
                let t = CGFloat(s) / CGFloat(segmentsPerSpan)
                result.append(catmull(p0, p1, p2, p3, t))
            }
        }
        result.append(closed ? controlPoints[0] : controlPoints[n - 1])
        return result
    }

    // MARK: - Геометрия дуги по трём точкам

    private func arcFromPoints(_ s: CGPoint, _ e: CGPoint, _ bulge: CGPoint) -> (center: CGPoint, radius: CGFloat, startAngle: CGFloat, endAngle: CGFloat)? {
        let mx = (s.x + e.x) / 2
        let my = (s.y + e.y) / 2
        let dx = e.x - s.x
        let dy = e.y - s.y
        let l = hypot(dx, dy)
        guard l > 1e-6 else { return nil }
        let nx = -dy / l
        let ny = dx / l
        let bx = bulge.x - mx
        let by = bulge.y - my
        let sag = bx * nx + by * ny
        let radius: CGFloat
        let cx: CGFloat
        let cy: CGFloat
        if abs(sag) < 1e-6 {
            radius = l / 2
            cx = mx - nx * radius
            cy = my - ny * radius
        } else {
            radius = (l * l / 4 + sag * sag) / (2 * sag)
            cx = mx - nx * (radius - sag)
            cy = my - ny * (radius - sag)
        }
        let center = CGPoint(x: cx, y: cy)
        let sa = atan2(s.y - cy, s.x - cx)
        var ea = atan2(e.y - cy, e.x - cx)
        let ba = atan2(bulge.y - cy, bulge.x - cx)
        var sweep = ea - sa
        while sweep <= 0 { sweep += 2 * .pi }
        while sweep > 2 * .pi { sweep -= 2 * .pi }
        var baRel = ba - sa
        while baRel < 0 { baRel += 2 * .pi }
        while baRel > 2 * .pi { baRel -= 2 * .pi }
        if baRel > sweep {
            sweep -= 2 * .pi
        }
        ea = sa + sweep
        return (center, abs(radius), sa, ea)
    }
}

// MARK: - Сэмплирование сплайна (Catmull-Rom)

/// Возвращает полилинию, аппроксимирующую кривую через контрольные точки.
/// Реализация — однородный Catmull-Rom; кривая проходит через все точки.
func sampleSpline(_ points: [CGPoint], closed: Bool, segmentsPerSpan: Int = 16) -> [CGPoint] {
    guard points.count >= 2 else { return points }
    let n = points.count
    let segs = max(1, segmentsPerSpan)
    var out: [CGPoint] = []
    out.reserveCapacity(n * segs)

    let get = { (i: Int) -> CGPoint in
        if closed {
            return points[(i % n + n) % n]
        }
        return points[min(max(i, 0), n - 1)]
    }

    let spanCount = closed ? n : n - 1
    for s in 0..<spanCount {
        let p0 = get(s - 1)
        let p1 = get(s)
        let p2 = get(s + 1)
        let p3 = get(s + 2)
        for j in 0..<segs {
            let t = Double(j) / Double(segs)
            let t2 = t * t
            let t3 = t2 * t
            let x = 0.5 * ((2 * p1.x)
                + (-p0.x + p2.x) * t
                + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2
                + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3)
            let y = 0.5 * ((2 * p1.y)
                + (-p0.y + p2.y) * t
                + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2
                + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3)
            out.append(CGPoint(x: x, y: y))
        }
    }
    out.append(points[closed ? 0 : n - 1])
    return out
}

// MARK: - Вспомогательные типы

struct SketchSnapCandidate: Equatable {
    let point: CGPoint
    let kind: SketchSnapKind
}

enum SketchSnapKind: Int {
    case endpoint = 0
    case midpoint = 1
    case center = 2

    var symbol: String {
        switch self {
        case .endpoint: return "●"
        case .midpoint: return "▱"
        case .center: return "◎"
        }
    }
}

private struct SketchSnapshot: Equatable {
    var entities: [SketchInputController.Entity]
    var constraints: [SketchInputController.Constraint]
}
