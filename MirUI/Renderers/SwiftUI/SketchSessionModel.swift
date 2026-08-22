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
    @Published private(set) var profileCount: UInt32 = 0
    @Published private(set) var inferredConstraints: [String] = []
    @Published private(set) var lastExtrudedID: UInt64 = 0

    /// Есть хотя бы один замкнутый профиль, пригодный для выдавливания.
    var hasValidProfile: Bool { profileCount > 0 }

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
        /// Отражение выбранной геометрии относительно оси X (y → -y).
    /// Одна запись History (TransformSelectionCommand в движке).
    func mirrorSelectionX() {
        guard let session, !selectedIDs.isEmpty else { return }
        _ = MirEngineSketchSessionMirrorSelection(session, 0, 0, 1, 0)
        commit()
    }

    /// Линейный массив выбранной геометрии: count копий со смещением (dx, dy).
        /// Линейный массив выбранной геометрии: count копий со смещением (dx, dy).
    func patternLinear(count: Int, dx: Double, dy: Double) {
        guard let session, count > 1, !selectedIDs.isEmpty else { return }
        _ = MirEngineSketchSessionPatternLinear(session, Int32(count), Float(dx), Float(dy))
        commit()
    }

    /// Круговой массив выбранной геометрии вокруг центра ( angleDegrees на шаг).
        /// Круговой массив выбранной геометрии вокруг центра (angleDegrees на шаг).
    func patternCircular(count: Int, center: CGPoint, angleDegrees: Double) {
        guard let session, count > 1, !selectedIDs.isEmpty else { return }
        _ = MirEngineSketchSessionPatternCircular(session, Int32(count), Float(center.x), Float(center.y), Float(angleDegrees))
        commit()
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
        profileCount = UInt32(state.profileCount)
        return (degreesOfFreedom, solverStatus)
    }

    func refreshHistoryState() {
        var state = MirEngineSketchSessionState()
        if let session {
            MirEngineSketchSessionGetState(session, &state)
        }
        canUndo = state.canUndo
        canRedo = state.canRedo
        profileCount = UInt32(state.profileCount)
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

    /// Центр выделенной геометрии (среднее точек вершин). Для массивов/зеркал.
    func selectionCenter() -> CGPoint {
        var sx = 0.0, sy = 0.0, n = 0
        for id in selectedIDs {
            for index in 0..<geometryCountValue() {
                guard geometryId(at: index) == id else { continue }
                switch geometryKind(at: index) {
                case .line:
                    if let l = line(at: index) { sx += l.start.x + l.end.x; sy += l.start.y + l.end.y; n += 2 }
                case .circle:
                    if let c = circle(at: index) { sx += c.center.x; sy += c.center.y; n += 1 }
                case .arc:
                    if let a = arc(at: index) { sx += a.center.x; sy += a.center.y; n += 1 }
                case .spline:
                    if let sp = spline(at: index) { for p in sp.points { sx += p.x; sy += p.y; n += 1 } }
                }
            }
        }
        guard n > 0 else { return .zero }
        return CGPoint(x: sx / Double(n), y: sy / Double(n))
    }

    /// Pick ближайшей геометрии (hit-test) через движок SketchSnapEngine.
    func pickGeometry(_ point: CGPoint, tolerance: Double = 10) -> UInt32? {
        guard let session else { return nil }
        var id: UInt32 = 0
        let ok = MirEngineSketchSessionPickGeometry(session,
                                                     Float(point.x), Float(point.y),
                                                     Float(tolerance), &id)
        return ok ? id : nil
    }

    /// Vertex-inference (Endpoint/Midpoint/Center) через движок SketchSnapEngine.
    func snapVertex(_ point: CGPoint, tolerance: Double = 6) -> CGPoint? {
        guard let session else { return nil }
        var ox: Float = 0, oy: Float = 0
        let ok = MirEngineSketchSessionSnapVertex(session,
                                                  Float(point.x), Float(point.y),
                                                  Float(tolerance), &ox, &oy)
        guard ok else { return nil }
        return CGPoint(x: Double(ox), y: Double(oy))
    }

    /// Смещение выбранной геометрии на расстояние distance (параллель/концентрично).
    /// Одна запись History.
    func offsetSelection(distance: Double) {
        guard let session, distance != 0, !selectedIDs.isEmpty else { return }
        _ = MirEngineSketchSessionOffsetSelection(session, Float(distance))
        commit()
    }
}
