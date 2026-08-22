import Foundation
import CoreGraphics
import SwiftUI

/// UI projection of the active MirEngine sketch session.
///
/// The C++ `SketchSession` remains the single source of truth. This type owns an
/// opaque engine session handle and exposes SwiftUI-friendly commands + state.
/// It deliberately stores NO engineering geometry — geometry is read back from
/// the engine on demand via the accessor methods.
@MainActor
final class SketchSessionModel: ObservableObject {
    @Published private(set) var solverStatus: String = "Не выполнен"
    @Published private(set) var degreesOfFreedom: Int?
    @Published private(set) var canUndo = false
    @Published private(set) var canRedo = false
    @Published private(set) var lastGeometryID: UInt32?
    @Published private(set) var geometryCount: UInt32 = 0
    @Published private(set) var constraintCount: UInt32 = 0
    @Published private(set) var inferredConstraints: [String] = []
    @Published private(set) var lastExtrudedID: UInt64 = 0

    private var session: OpaquePointer?

    // Backwards-compatible closure-based wiring (unused when the engine session
    // is available). Kept so existing composition roots keep compiling.
    private let createLineHandler: ((CGPoint, CGPoint) -> SketchLineCommandResult)?
    private let undoHandler: (() -> Bool)?
    private let redoHandler: (() -> Bool)?

    init(
        createLine: ((CGPoint, CGPoint) -> SketchLineCommandResult)? = nil,
        undo: (() -> Bool)? = nil,
        redo: (() -> Bool)? = nil
    ) {
        self.createLineHandler = createLine
        self.undoHandler = undo
        self.redoHandler = redo
        self.session = MirEngineSketchSessionCreate()
        refreshHistoryState()
    }

    @MainActor deinit {
        if let session {
            MirEngineSketchSessionDestroy(session)
        }
    }

    // MARK: - Geometry creation (engine-backed)

    @discardableResult
    func createLine(from start: CGPoint, to end: CGPoint) -> SketchLineCommandResult? {
        guard let session else {
            return createLineHandler.map { $0(start, end) }
        }
        let id = MirEngineSketchSessionCreateLine(
            session,
            Float(start.x), Float(start.y),
            Float(end.x), Float(end.y))
        lastGeometryID = id
        let (dof, status) = commit()
        return SketchLineCommandResult(
            success: id != 0,
            geometryID: id,
            constraintIDs: [],
            inferredConstraints: inferredConstraints,
            degreesOfFreedom: dof,
            solverStatus: status)
    }

    @discardableResult
    func createCircle(center: CGPoint, radius: Double) -> UInt32 {
        guard let session else { return 0 }
        let id = MirEngineSketchSessionCreateCircle(session, Float(center.x), Float(center.y), Float(radius))
        lastGeometryID = id
        commit()
        return id
    }

    @discardableResult
    func createArc(center: CGPoint, radius: Double, startAngle: Double, endAngle: Double) -> UInt32 {
        guard let session else { return 0 }
        let id = MirEngineSketchSessionCreateArc(
            session, Float(center.x), Float(center.y), Float(radius),
            Float(startAngle), Float(endAngle))
        lastGeometryID = id
        commit()
        return id
    }

    func createSpline(points: [CGPoint], closed: Bool = false) -> UInt32 {
        guard let session, points.count >= 2 else { return 0 }
        let xs = points.map { Float($0.x) }
        let ys = points.map { Float($0.y) }
        let count = UInt32(points.count)
        let id = xs.withUnsafeBufferPointer { xb in
            ys.withUnsafeBufferPointer { yb in
                MirEngineSketchSessionCreateSpline(
                    session, xb.baseAddress, yb.baseAddress, count, closed)
            }
        }
        lastGeometryID = id
        commit()
        return id
    }

    /// Отражает выбранную геометрию относительно оси X (y → -y).
    /// Создаёт зеркальные копии через штатные команды создания.
    func mirrorSelectionX() {
        let ids = selectedIDs
        guard !ids.isEmpty else { return }
        for id in ids {
            for index in 0..<geometryCountValue() {
                guard geometryId(at: index) == id else { continue }
                switch geometryKind(at: index) {
                case .line:
                    if let l = line(at: index) {
                        createLine(from: CGPoint(x: l.start.x, y: -l.start.y),
                                   to: CGPoint(x: l.end.x, y: -l.end.y))
                    }
                case .circle:
                    if let c = circle(at: index) {
                        createCircle(center: CGPoint(x: c.center.x, y: -c.center.y), radius: c.radius)
                    }
                case .arc:
                    if let a = arc(at: index) {
                        createArc(center: CGPoint(x: a.center.x, y: -a.center.y),
                                  radius: a.radius,
                                  startAngle: -a.end,
                                  endAngle: -a.start)
                    }
                case .spline:
                    break
                }
            }
        }
        clearSelection()
    }

    /// Линейный массив выбранной геометрии: count копий со смещением (dx, dy).
    func patternLinear(count: Int, dx: Double, dy: Double) {
        let ids = selectedIDs
        guard count > 1, !ids.isEmpty else { return }
        for id in ids {
            for index in 0..<geometryCountValue() {
                guard geometryId(at: index) == id else { continue }
                for k in 1..<count {
                    let ox = dx * Double(k)
                    let oy = dy * Double(k)
                    switch geometryKind(at: index) {
                    case .line:
                        if let l = line(at: index) {
                            createLine(from: CGPoint(x: l.start.x + ox, y: l.start.y + oy),
                                       to: CGPoint(x: l.end.x + ox, y: l.end.y + oy))
                        }
                    case .circle:
                        if let c = circle(at: index) {
                            createCircle(center: CGPoint(x: c.center.x + ox, y: c.center.y + oy),
                                         radius: c.radius)
                        }
                    case .arc:
                        if let a = arc(at: index) {
                            createArc(center: CGPoint(x: a.center.x + ox, y: a.center.y + oy),
                                      radius: a.radius, startAngle: a.start, endAngle: a.end)
                        }
                    case .spline:
                        break
                    }
                }
            }
        }
        clearSelection()
    }

    /// Круговой массив выбранной геометрии вокруг центра ( angleDegrees на шаг).
    func patternCircular(count: Int, center: CGPoint, angleDegrees: Double) {
        let ids = selectedIDs
        guard count > 1, !ids.isEmpty else { return }
        let base = angleDegrees * .pi / 180
        for id in ids {
            for index in 0..<geometryCountValue() {
                guard geometryId(at: index) == id else { continue }
                for k in 1..<count {
                    let ang = base * Double(k)
                    let cs = cos(ang)
                    let sn = sin(ang)
                    let rot: (CGPoint) -> CGPoint = { p in
                        let x = p.x - center.x
                        let y = p.y - center.y
                        return CGPoint(x: center.x + x * cs - y * sn,
                                       y: center.y + x * sn + y * cs)
                    }
                    switch geometryKind(at: index) {
                    case .line:
                        if let l = line(at: index) {
                            createLine(from: rot(l.start), to: rot(l.end))
                        }
                    case .circle:
                        if let c = circle(at: index) {
                            createCircle(center: rot(c.center), radius: c.radius)
                        }
                    case .arc:
                        if let a = arc(at: index) {
                            createArc(center: rot(a.center),
                                      radius: a.radius,
                                      startAngle: a.start + ang,
                                      endAngle: a.end + ang)
                        }
                    case .spline:
                        break
                    }
                }
            }
        }
        clearSelection()
    }

    @discardableResult
    func createRectangle(from start: CGPoint, to end: CGPoint) -> UInt32 {
        guard let session else { return 0 }
        let id = MirEngineSketchSessionCreateRectangle(
            session, Float(start.x), Float(start.y), Float(end.x), Float(end.y))
        lastGeometryID = id
        commit()
        return id
    }

    func deleteGeometry(id: UInt32) {
        guard let session, id != 0 else { return }
        _ = MirEngineSketchSessionDeleteGeometry(session, id)
        commit()
    }

    /// Removes every geometry entity (and its constraints) from the session.
    func clearAll() {
        guard let session else { return }
        let ids = (0..<MirEngineSketchSessionGeometryCount(session))
            .map { MirEngineSketchSessionGeometryIdAt(session, $0) }
        for id in ids where id != 0 {
            _ = MirEngineSketchSessionDeleteGeometry(session, id)
        }
        commit()
    }

    // MARK: - Constraints

    @discardableResult
    func addConstraint(type: MirEngineSketchConstraint, geometry: UInt32, target: UInt32 = 0, value: Double = 0) -> UInt32 {
        guard let session else { return 0 }
        let id = MirEngineSketchSessionAddConstraint(session, type.rawValue, geometry, target, value)
        commit()
        return id
    }

    func removeConstraint(id: UInt32) {
        guard let session, id != 0 else { return }
        _ = MirEngineSketchSessionRemoveConstraint(session, id)
        commit()
    }

    // MARK: - History

    @discardableResult
    func undo() -> Bool {
        if let session {
            let ok = MirEngineSketchSessionUndo(session)
            if ok { refreshHistoryState() }
            return ok
        }
        let success = undoHandler?() ?? false
        if success { refreshHistoryState() }
        return success
    }

    @discardableResult
    func redo() -> Bool {
        if let session {
            let ok = MirEngineSketchSessionRedo(session)
            if ok { refreshHistoryState() }
            return ok
        }
        let success = redoHandler?() ?? false
        if success { refreshHistoryState() }
        return success
    }

    // MARK: - Selection

    func select(geometryID: UInt32, additive: Bool = false) {
        guard let session else { return }
        MirEngineSketchSessionSelect(session, geometryID, additive)
        refreshHistoryState()
    }

    func clearSelection() {
        guard let session else { return }
        MirEngineSketchSessionClearSelection(session)
        refreshHistoryState()
    }

    // MARK: - Extrude

    /// Выдавливает текущий профиль сессии в 3D-сцену viewport.
    /// `viewport` — нетипизированный указатель 3D-вида; если nil, профиль
    /// не может быть помещён в сцену (возвращает 0).
    func extrude(
        distance: Double,
        viewport: UnsafeMutableRawPointer? = nil,
        origin: (x: Double, y: Double, z: Double) = (0, 0, 0),
        normal: (x: Double, y: Double, z: Double) = (0, 0, 1),
        xAxis: (x: Double, y: Double, z: Double) = (1, 0, 0),
        yAxis: (x: Double, y: Double, z: Double) = (0, 1, 0)
    ) -> UInt64 {
        guard let session, distance > 0 else { return 0 }
        let id = MirEngineSketchSessionExtrude(
            session, viewport, distance,
            origin.x, origin.y, origin.z,
            normal.x, normal.y, normal.z,
            xAxis.x, xAxis.y, xAxis.z,
            yAxis.x, yAxis.y, yAxis.z)
        lastExtrudedID = id
        return id
    }

    var selectedIDs: [UInt32] {
        guard let session else { return [] }
        let count = MirEngineSketchSessionSelectedCount(session)
        return (0..<count).compactMap { MirEngineSketchSessionSelectedAt(session, $0) }
    }

    // MARK: - Geometry read-back (SwiftUI renders from here, never stores it)

    func geometryCountValue() -> UInt32 {
        guard let session else { return 0 }
        return MirEngineSketchSessionGeometryCount(session)
    }

    func geometryKind(at index: UInt32) -> MirEngineSketchGeometryKind {
        guard let session else { return .line }
        return MirEngineSketchGeometryKind(rawValue: MirEngineSketchSessionGeometryTypeAt(session, index)) ?? .line
    }

    func geometryId(at index: UInt32) -> UInt32 {
        guard let session else { return 0 }
        return MirEngineSketchSessionGeometryIdAt(session, index)
    }

    func line(at index: UInt32) -> (start: CGPoint, end: CGPoint)? {
        guard let session else { return nil }
        var x1: Float = 0, y1: Float = 0, x2: Float = 0, y2: Float = 0
        guard MirEngineSketchSessionLineAt(session, index, &x1, &y1, &x2, &y2) else { return nil }
        return (CGPoint(x: Double(x1), y: Double(y1)), CGPoint(x: Double(x2), y: Double(y2)))
    }

    func circle(at index: UInt32) -> (center: CGPoint, radius: Double)? {
        guard let session else { return nil }
        var cx: Float = 0, cy: Float = 0, r: Float = 0
        guard MirEngineSketchSessionCircleAt(session, index, &cx, &cy, &r) else { return nil }
        return (CGPoint(x: Double(cx), y: Double(cy)), Double(r))
    }

    func arc(at index: UInt32) -> (center: CGPoint, radius: Double, start: Double, end: Double)? {
        guard let session else { return nil }
        var cx: Float = 0, cy: Float = 0, r: Float = 0, sa: Float = 0, ea: Float = 0
        guard MirEngineSketchSessionArcAt(session, index, &cx, &cy, &r, &sa, &ea) else { return nil }
        return (CGPoint(x: Double(cx), y: Double(cy)), Double(r), Double(sa), Double(ea))
    }

    func spline(at index: UInt32) -> (points: [CGPoint], closed: Bool)? {
        guard let session else { return nil }
        var count: UInt32 = 0
        var closed = false
        guard MirEngineSketchSessionSplineAt(session, index, nil, nil, &count, &closed) else { return nil }
        guard count > 0 else { return (points: [], closed: closed) }
        var xs = [Float](repeating: 0, count: Int(count))
        var ys = [Float](repeating: 0, count: Int(count))
        let ok = xs.withUnsafeMutableBufferPointer { xb in
            ys.withUnsafeMutableBufferPointer { yb in
                MirEngineSketchSessionSplineAt(session, index, xb.baseAddress, yb.baseAddress, &count, &closed)
            }
        }
        guard ok else { return nil }
        let pts = (0..<Int(count)).map { CGPoint(x: CGFloat(xs[$0]), y: CGFloat(ys[$0])) }
        return (points: pts, closed: closed)
    }

    func constraintCountValue() -> UInt32 {
        guard let session else { return 0 }
        return MirEngineSketchSessionConstraintCount(session)
    }

    func constraint(at index: UInt32) -> (type: MirEngineSketchConstraint, first: UInt32, second: UInt32, value: Double)? {
        guard let session else { return nil }
        var type: Int32 = 0, g1: UInt32 = 0, g2: UInt32 = 0
        var value: Double = 0
        guard MirEngineSketchSessionConstraintAt(session, index, &type, &g1, &g2, &value) else { return nil }
        return (MirEngineSketchConstraint(rawValue: type) ?? .coincident, g1, g2, value)
    }

    // MARK: - State sync

    /// Solves the document (writing results into the engine) and refreshes the
    /// published UI state. Every committed operation should call this.
    @discardableResult
    private func commit() -> (degreesOfFreedom: Int?, solverStatus: String) {
        if let session {
            _ = MirEngineSketchSessionSolve(session)
        }
        var state = MirEngineSketchSessionState()
        if let session {
            MirEngineSketchSessionGetState(session, &state)
        }
        degreesOfFreedom = state.degreesOfFreedom < 0 ? nil : Int(state.degreesOfFreedom)
        solverStatus = SketchSessionModel.solverStatusString(state.solverStatus)
        canUndo = state.canUndo
        canRedo = state.canRedo
        geometryCount = UInt32(state.geometryCount)
        constraintCount = UInt32(state.constraintCount)
        return (degreesOfFreedom, solverStatus)
    }

    func refreshHistoryState() {
        var state = MirEngineSketchSessionState()
        if let session {
            MirEngineSketchSessionGetState(session, &state)
        }
        canUndo = state.canUndo
        canRedo = state.canRedo
    }

    private static func solverStatusString(_ raw: Int32) -> String {
        switch MirEngineSketchSolverStatus(rawValue: raw) ?? .notRun {
        case .notRun: return "Не выполнен"
        case .solved: return "Решено"
        case .underConstrained: return "Недоопределён"
        case .overConstrained: return "Переопределён"
        case .failed: return "Ошибка решателя"
        }
    }

    /// Смещение выбранной геометрии на расстояние distance (параллель/концентрично).
    func offsetSelection(distance: Double) {
        let ids = selectedIDs
        guard distance != 0, !ids.isEmpty else { return }
        for id in ids {
            for index in 0..<geometryCountValue() {
                guard geometryId(at: index) == id else { continue }
                switch geometryKind(at: index) {
                case .line:
                    if let l = line(at: index) {
                        let dx = l.end.x - l.start.x
                        let dy = l.end.y - l.start.y
                        let len = hypot(dx, dy)
                        guard len > 1e-9 else { continue }
                        let nx = -dy / len
                        let ny = dx / len
                        let ox = nx * distance
                        let oy = ny * distance
                        createLine(from: CGPoint(x: l.start.x + ox, y: l.start.y + oy),
                                   to: CGPoint(x: l.end.x + ox, y: l.end.y + oy))
                    }
                case .circle:
                    if let c = circle(at: index) {
                        let r = c.radius + distance
                        guard r > 0 else { continue }
                        createCircle(center: c.center, radius: r)
                    }
                case .arc:
                    if let a = arc(at: index) {
                        let r = a.radius + distance
                        guard r > 0 else { continue }
                        createArc(center: a.center, radius: r, startAngle: a.start, endAngle: a.end)
                    }
                case .spline:
                    break
                }
            }
        }
        clearSelection()
    }
}
