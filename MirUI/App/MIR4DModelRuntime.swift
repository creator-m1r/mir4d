import Foundation
import Combine
import SwiftUI
import MirUIHandGesture

extension Notification.Name {
    static let mir4DModelChanged = Notification.Name("MIR4D.ModelChanged")
    static let mir4DEngineDocumentChanged = Notification.Name("MIR4D.EngineDocumentChanged")
    static let mir4DRunCAECampaign = Notification.Name("MIR4D.RunCAECampaign")
    static let mir4DCAEResult = Notification.Name("MIR4D.CAEResult")
}

/// Live parameter/document model used by the SwiftUI workspace.
/// The C++ bridge is the authoritative evaluated MirEngine document.
@MainActor
final class MIR4DModelRuntime: ObservableObject {
    static let shared = MIR4DModelRuntime()

    @Published private(set) var document: MIR4DModelDocument
    @Published private(set) var revision: UInt64 = 0
    @Published private(set) var engineObjectCount: Int = 0
    @Published private(set) var engineCommandCount: Int = 0
    @Published private(set) var engineRevision: UInt64 = 0
    @Published private(set) var engineIsValid: Bool = true
    @Published private(set) var engineIsModified: Bool = false

#if !MIR4D_SWIFTPM
    // C handle owned exclusively on the main actor. It is marked
    // nonisolated(unsafe) so the deinit can release it; the instance is a
    // process-lifetime singleton, so no cross-thread access can occur.
    nonisolated(unsafe) private var engineDocument: UnsafeMutableRawPointer?
#endif

    /// Live viewport engine handle published by MirGLView. Used by the CAE
    /// geometry bridge to read real metrics of the selected CAD object.
    nonisolated(unsafe) var viewport: UnsafeMutableRawPointer?

    /// Live OpenGL renderer handle published by MirGLView. Required to push the
    /// 2D sketch overlay (and other immediate-mode draws) from App-layer bridges.
    nonisolated(unsafe) var renderer: UnsafeMutableRawPointer?

    /// Active sketch plane basis mirrored from `CADAppState.sketchPlane`, used by
    /// the sketch command bridge to orient hand-drawn strokes onto the chosen
    /// work plane (XY/YZ/ZX) instead of a fixed base plane.
    nonisolated(unsafe) var activeSketchPlane: SketchPlaneAnchor?

    /// Closed-loop hand grab controller (Vertical Slice v0.1). It subscribes to
    /// the hand intent stream and drives the real MirEngine through this runtime;
    /// when no viewport is present its calls are safe no-ops.
    private let handGrabController = MIRHandGrabController()

    /// Подписки на потоки данных hand-подсистемы (скелет и т.п.).
    private var cancellables = Set<AnyCancellable>()

    /// Состояние моста визуализации скелета.
    private var handSkeletonTopologySent = false
    private var lastSkeletonStyleSignature: String?
    private var lastSkeletonDataSignature: String?

    /// Runtime-only mapping from persisted geometry identity to the fresh
    /// MirEngine object identity created during evaluation. Engine IDs are not
    /// persisted because the engine document is rebuilt when a project opens.
    private var engineObjectIDs: [UUID: UInt64] = [:]

    /// Maps a persisted body UUID to the runtime viewport engine object ID.
    /// The viewport scene owns a separate registry, so its selection IDs must
    /// be bridged back to persisted model identity without inventing a second
    /// CAD ID scheme.
    private var viewportObjectIDs: [UUID: UInt64] = [:]

    /// Registers the viewport engine object identity created for a body.
    func registerViewportEngineID(bodyID: UUID, engineObjectID: UInt64) {
        guard engineObjectID > 0 else { return }
        viewportObjectIDs[bodyID] = engineObjectID
    }

    /// Applies an in-place sculpt stroke to the selected object through MirEngine.
    /// `x,y,z` and `radius` are in CAD world units; `strength` is a signed
    /// displacement magnitude; `mode` matches `MIR4DSculptIntent.Mode` ordering.
    @discardableResult
    func deformSelected(x: Double, y: Double, z: Double, radius: Double, strength: Double, mode: Int) -> Bool {
        guard let viewport else { return false }
        return mirEngineDeformSelected(viewport, x, y, z, radius, strength, Int32(mode))
    }

    /// Snapshots the selected mesh so a subsequent `deformSelected` stroke is a
    /// single undoable command. Pairs with `endDeformStroke`.
    func beginDeformStroke() {
        guard let viewport else { return }
        _ = mirEngineBeginDeformSelected(viewport)
    }

    /// Commits one undoable deform command for the active stroke. Pairs with
    /// `beginDeformStroke`.
    func endDeformStroke() {
        guard let viewport else { return }
        _ = mirEngineEndDeformSelected(viewport)
    }

    /// Resolves a persisted body UUID from a viewport engine object ID.
    func persistedBodyID(forViewportEngineObjectID objectID: UInt64) -> UUID? {
        guard objectID > 0 else { return nil }
        for (bodyID, viewportID) in viewportObjectIDs where viewportID == objectID {
            return bodyID
        }
        return nil
    }

    /// Selects a persisted body in the MirEngine viewport by its UUID so that
    /// selection-driven tools (sculpt) work even when the body was picked from
    /// the CAD tree rather than the 3D viewport (which already sets it).
    func selectBody(persistedID: UUID) {
        guard let viewport else { return }
        guard let objectID = viewportObjectIDs[persistedID], objectID > 0 else { return }
        mirEngineSelectObject(viewport, objectID)
    }

    /// Ray-casts the viewport camera through normalized screen coords (nx, ny in
    /// -1..1, screen-centred, y up) and returns the world hit point plus the hit
    /// engine object id. Used to place the air-sculpt brush where the hand points,
    /// regardless of camera orientation. Returns nil on a miss.
    func pickWorldPoint(nx: Double, ny: Double) -> (point: SIMD3<Double>, objectId: UInt64)? {
        guard let viewport else { return nil }
        var x = 0.0, y = 0.0, z = 0.0
        var objectId: UInt64 = 0
        guard mirEnginePickWorldPoint(viewport, nx, ny, &x, &y, &z, &objectId) else { return nil }
        return (SIMD3(x, y, z), objectId)
    }

    // MARK: - Hand Grab (Vertical Slice v0.1: Pinch → point → grab → move → commit)

    /// World-space camera eye, used by the hand-grab controller to build the
    /// picking ray (eye → calibrated hand point).
    func cameraEye() -> SIMD3<Double>? {
        guard let viewport else { return nil }
        var x = 0.0, y = 0.0, z = 0.0
        guard MirEngineGetCameraEye(viewport, &x, &y, &z) else { return nil }
        return SIMD3(x, y, z)
    }

    /// Casts a world-space hand ray and returns the first hit object id.
    func pickHandRay(origin: SIMD3<Double>, direction: SIMD3<Double>) -> (objectId: UInt64, distance: Double)? {
        guard let viewport else { return nil }
        var objectId: UInt64 = 0
        var distance = 0.0
        guard MirEnginePickHandRay(viewport, origin.x, origin.y, origin.z,
                                   direction.x, direction.y, direction.z,
                                   &objectId, &distance) else { return nil }
        return (objectId, distance)
    }

    /// Arms a grab on `objectId`, snapshots its transform and selects it.
    func beginGrab(objectId: UInt64) {
        guard let viewport, objectId > 0 else { return }
        MirEngineBeginGrab(viewport, objectId)
    }

    /// Live preview of the grabbed object transform (no history entry yet).
    func previewGrab(objectId: UInt64, transform: MirTransform) {
        guard let viewport, objectId > 0 else { return }
        _ = MirEnginePreviewGrab(viewport, objectId, transform)
    }

    /// Commits exactly one undoable transform command; returns false if nothing moved.
    @discardableResult
    func commitGrab(objectId: UInt64) -> Bool {
        guard let viewport, objectId > 0 else { return false }
        return MirEngineCommitGrab(viewport, objectId)
    }

    /// Cancels the active grab, restoring the snapshot transform.
    func cancelGrab() {
        guard let viewport else { return }
        MirEngineCancelGrab(viewport)
    }

    /// Current world transform of an object (seed for preview deltas).
    func getObjectTransform(objectId: UInt64) -> MirTransform? {
        guard let viewport, objectId > 0 else { return nil }
        var t = MirTransform()
        guard MirEngineGetObjectTransform(viewport, objectId, &t) else { return nil }
        return t
    }

    /// Highlights the object currently under the hand (hover, no selection change).
    func setHandHover(objectId: UInt64) {
        guard let viewport, objectId > 0 else { return }
        MirEngineSetHandHover(viewport, objectId)
    }

    /// Передаёт кадры скелета кистей в растератор (отдельный debug / assist
    /// режим). Суставы в кадре — в порядке `LandmarkID.allCases`, что совпадает
    /// с индексами костей на стороне C++ (`HandSkeletonPass`). Ничего не меняет
    /// в CAD-сцене, Document или History.
    func setHandSkeleton(frames: [MIRHandSkeletonFrame]) {
        guard let viewport else { return }
        let mode = MIRHandGestureModule.shared.configuration.skeletonVisualizationMode
        guard mode != .off, !frames.isEmpty else {
            clearHandSkeleton()
            return
        }

        // Топология — единый источник (Swift MIRHandSkeletonTopology). Шлём один раз.
        if !handSkeletonTopologySent {
            let topo = MIRHandSkeletonBuilder.topologyIndices()
            topo.withUnsafeBufferPointer { buf in
                MirEngineSetHandSkeletonTopology(viewport, Int32(topo.count / 2), buf.baseAddress)
            }
            handSkeletonTopologySent = true
        }

        let handCount = min(frames.count, 2)
        var positions = [Double]()
        var confidence = [Double]()
        var handedness = [Int32]()
        var pinch = [Double]()
        var gesture = [Int32]()
        positions.reserveCapacity(handCount * 21 * 3)
        confidence.reserveCapacity(handCount * 21)
        handedness.reserveCapacity(handCount)
        pinch.reserveCapacity(handCount)
        gesture.reserveCapacity(handCount)
        for h in 0..<handCount {
            let frame = frames[h]
            for j in 0..<21 {
                if j < frame.joints.count {
                    let p = frame.joints[j].position
                    positions.append(contentsOf: [p.x, p.y, p.z])
                    confidence.append(frame.joints[j].confidence)
                } else {
                    positions.append(contentsOf: [0.0, 0.0, 0.0])
                    confidence.append(0.0)
                }
            }
            handedness.append(frame.handedness == .right ? 1 : (frame.handedness == .left ? 0 : 2))
            pinch.append(frame.pinch)
            gesture.append(MIRHandSkeletonBuilder.gestureCode(frame.gesture))
        }
        let modeRaw = Int32(mode.rawValue)
        let hc = Int32(handCount)
        positions.withUnsafeBufferPointer { pBuf in
            confidence.withUnsafeBufferPointer { cBuf in
                handedness.withUnsafeBufferPointer { hBuf in
                    pinch.withUnsafeBufferPointer { pinBuf in
                        gesture.withUnsafeBufferPointer { gBuf in
                            MirEngineSetHandSkeleton(
                                viewport, modeRaw, hc,
                                pBuf.baseAddress, cBuf.baseAddress, hBuf.baseAddress,
                                pinBuf.baseAddress, gBuf.baseAddress)
                        }
                    }
                }
            }
        }
    }

    /// Очищает оверлей скелета в растераторе.
    func clearHandSkeleton() {
        guard let viewport else { return }
        MirEngineClearHandSkeleton(viewport)
    }

    /// Передаёт стиль оверлея скелета в движок (цвета/размеры/глубина).
    /// Вызывается каждый тик, но реальный C-вызов только при изменении конфигурации.
    private func pushHandSkeletonStyleIfNeeded() {
        guard let viewport else { return }
        let c = MIRHandGestureModule.shared.configuration
        let sig = "\(c.skeletonLeftColor)|\(c.skeletonRightColor)|\(c.skeletonJointSize)|"
                  + "\(c.skeletonTipSize)|\(c.skeletonWristSize)|\(c.skeletonAlpha)|\(c.skeletonDepthTest)"
        if sig == lastSkeletonStyleSignature { return }
        lastSkeletonStyleSignature = sig
        MirEngineSetHandSkeletonStyle(
            viewport,
            Float(c.skeletonLeftColor.x), Float(c.skeletonLeftColor.y), Float(c.skeletonLeftColor.z),
            Float(c.skeletonRightColor.x), Float(c.skeletonRightColor.y), Float(c.skeletonRightColor.z),
            Float(c.skeletonJointSize), Float(c.skeletonTipSize), Float(c.skeletonWristSize),
            Float(c.skeletonAlpha), c.skeletonDepthTest ? 1 : 0)
    }

    /// Лёгкая сигнатура кадра для throttle: меняется, когда рука движется
    /// или переключается режим, — иначе повторная отправка пропускается.
    private func skeletonSignature(frames: [MIRHandSkeletonFrame],
                                   mode: MIRHandSkeletonVisMode) -> String {
        var s = "\(mode.rawValue)|\(frames.count)"
        for f in frames {
            s += "|\(f.handedness.rawValue):\(Int((f.pinch * 100).rounded()))"
            if let j = f.joints.first {
                s += ":\(Int((j.position.x * 1000).rounded()))"
            }
        }
        return s
    }

    /// Removes a body (with its operations and geometry) from the persisted    /// model after the MirEngine scene deleted the object. MirEngine Scene is
    /// the source of truth; this keeps the navigation tree in sync.
    @discardableResult
    func removeBody(forViewportObjectID objectID: UInt64) -> Bool {
        guard let bodyID = persistedBodyID(forViewportEngineObjectID: objectID) else {
            return false
        }

        let operationIDs = document.operations
            .filter { $0.bodyID == bodyID }
            .map(\.id)
        let geometryIDs = document.geometry
            .filter { geometry in
                guard let operationID = geometry.operationID else { return false }
                return operationIDs.contains(operationID)
            }
            .map(\.id)

        document.geometry.removeAll { geometry in
            geometryIDs.contains(geometry.id)
        }
        document.operations.removeAll { $0.bodyID == bodyID }
        document.bodies.removeAll { $0.id == bodyID }
        document.root.removeChild(id: bodyID)

        for geometryID in geometryIDs {
            engineObjectIDs.removeValue(forKey: geometryID)
        }
        viewportObjectIDs.removeValue(forKey: bodyID)

        revision &+= 1
        publishChange()
        return true
    }

    /// Переименовывает тело модели по его идентификатору (для совместной работы).
    func renameBody(_ bodyID: UUID, to name: String) {
        guard let index = document.bodies.firstIndex(where: { $0.id == bodyID }) else { return }
        document.bodies[index].name = name
        if let nodeIndex = document.root.children.firstIndex(where: { $0.id == bodyID }) {
            document.root.children[nodeIndex].title = name
        }
        revision &+= 1
        publishChange()
    }

    /// Применяет матрицу преобразования (4x4, построчно) к телу модели.
    /// Матрица хранится в CRDT совместной работы; здесь фиксируется факт
    /// изменения для синхронизации дерева проекта.
    func applyCollaborationTransform(bodyID: UUID, matrix: [Double]) {
        _ = matrix
        guard document.operations.contains(where: { $0.bodyID == bodyID }) else { return }
        revision &+= 1
        publishChange()
    }

    private init() {
        document = MIR4DModelDocument.newProject(name: "Новый проект")
#if !MIR4D_SWIFTPM
        engineDocument = MIR4DDocumentCreate()
        syncEngineState()
#endif
        handGrabController.start()

        // Мост визуализации скелета кистей: кадры из сессии → растератор.
        // Режим выключен по умолчанию, поэтому без активного режима оверлей
        // не загружается (нулевая стоимость).
        MIRHandGestureModule.shared.session.$skeletonFrames
            .receive(on: DispatchQueue.main)
            .sink { [weak self] frames in
                guard let self else { return }
                if frames.isEmpty ||
                    MIRHandGestureModule.shared.configuration.skeletonVisualizationMode == .off {
                    self.clearHandSkeleton()
                } else {
                    self.setHandSkeleton(frames: frames)
                }
            }
            .store(in: &cancellables)
    }

    deinit {
#if !MIR4D_SWIFTPM
        if let engineDocument { MIR4DDocumentDestroy(engineDocument) }
#endif
    }

    func reset(projectName: String) {
        document = MIR4DModelDocument.newProject(name: projectName)
        engineObjectIDs.removeAll(keepingCapacity: true)
        viewportObjectIDs.removeAll(keepingCapacity: true)
#if !MIR4D_SWIFTPM
        if engineDocument == nil { engineDocument = MIR4DDocumentCreate() }
        projectName.withCString { MIR4DDocumentReset(engineDocument, $0) }
        syncEngineState()
#endif
        revision &+= 1
        publishChange()
    }

    func load(_ document: MIR4DModelDocument) {
        self.document = document
        engineObjectIDs.removeAll(keepingCapacity: true)
        viewportObjectIDs.removeAll(keepingCapacity: true)
#if !MIR4D_SWIFTPM
        if engineDocument == nil { engineDocument = MIR4DDocumentCreate() }
        document.root.title.withCString { MIR4DDocumentReset(engineDocument, $0) }

        // Rebuild the evaluated MirEngine document from the persisted
        // parameter model. The renderer receives the fresh runtime IDs below.
        for geometry in document.geometry where geometry.kind == .box {
            let width = geometry.parameters["width"] ?? 0
            let depth = geometry.parameters["depth"] ?? 0
            let height = geometry.parameters["height"] ?? 0
            guard width > 0, depth > 0, height > 0 else { continue }

            var objectID: UInt64 = 0
            guard MIR4DDocumentCreateBox(engineDocument, width, depth, height, &objectID) else {
                continue
            }
            engineObjectIDs[geometry.id] = objectID
        }
        syncEngineState()
#endif
        revision &+= 1
        publishChange()
        replayGeometryToViewport()
    }

    @discardableResult
    func addBox(width: Double, depth: Double, height: Double, engineObjectID: UInt64? = nil, bodyID: UUID? = nil) -> UUID {
        var resolvedEngineObjectID = engineObjectID
#if !MIR4D_SWIFTPM
        if resolvedEngineObjectID == nil {
            var objectID: UInt64 = 0
            guard MIR4DDocumentCreateBox(engineDocument, width, depth, height, &objectID) else {
                return UUID()
            }
            resolvedEngineObjectID = objectID
        }
#endif
        let bodyID = bodyID ?? UUID()
        let operationID = UUID()
        let geometryID = UUID()
        let body = MIR4DBody(id: bodyID, name: "Тело \(document.bodies.count + 1)")
        let operation = MIR4DOperation(
            id: operationID,
            name: "Параллелепипед \(document.operations.count + 1)",
            kind: .extrude,
            bodyID: bodyID,
            parameters: ["width": width, "depth": depth, "height": height],
            featureIDs: [geometryID]
        )
        var parameters: [String: Double] = [
            "width": width,
            "depth": depth,
            "height": height
        ]
        if let resolvedEngineObjectID {
            parameters["engineObjectID"] = Double(resolvedEngineObjectID)
        }
        let geometry = MIR4DGeometry(
            id: geometryID,
            kind: .box,
            operationID: operationID,
            parameters: parameters,
            source: "MirEngine"
        )
        let operationNode = MIR4DModelNode(
            id: operationID,
            title: operation.name,
            kind: .operation,
            children: [MIR4DModelNode(id: geometryID, title: "Геометрия", kind: .result)]
        )
        let bodyNode = MIR4DModelNode(
            id: bodyID,
            title: body.name,
            kind: .body,
            children: [operationNode]
        )

        document.bodies.append(body)
        document.operations.append(operation)
        document.geometry.append(geometry)
        document.root.children.append(bodyNode)

        if let resolvedEngineObjectID {
            engineObjectIDs[geometryID] = resolvedEngineObjectID
        }

        revision &+= 1
#if !MIR4D_SWIFTPM
        syncEngineState()
#endif
        publishChange()
        return bodyID
    }

    private func replayGeometryToViewport() {
        let payloads = document.geometry.compactMap { geometry -> [String: Any]? in
            guard geometry.kind == .box else { return nil }
            guard let width = geometry.parameters["width"],
                  let depth = geometry.parameters["depth"],
                  let height = geometry.parameters["height"],
                  let operationID = geometry.operationID,
                  width > 0, depth > 0, height > 0 else { return nil }

            let operation = document.operations.first { $0.id == operationID }
            let bodyID = operation?.bodyID

            var payload: [String: Any] = [
                "width": width,
                "depth": depth,
                "height": height,
                "geometryID": geometry.id.uuidString,
                "operationID": operationID.uuidString
            ]
            if let bodyID = bodyID {
                payload["bodyID"] = bodyID.uuidString
            }
            if let engineObjectID = engineObjectIDs[geometry.id] {
                payload["engineObjectID"] = engineObjectID
            }
            return payload
        }

        guard !payloads.isEmpty else { return }

        // CADMainView creates the native OpenGL view immediately after the
        // project is loaded. Replay on the next run-loop turn so the renderer
        // can consume the evaluated geometry during workspace hydration.
        DispatchQueue.main.async { [weak self] in
            guard self != nil else { return }
            for payload in payloads {
                NotificationCenter.default.post(
                    name: .mir4DEngineDocumentChanged,
                    object: nil,
                    userInfo: ["geometry": payload]
                )
            }
        }
    }

#if !MIR4D_SWIFTPM
    private func syncEngineState() {
        engineObjectCount = Int(MIR4DDocumentObjectCount(engineDocument))
        engineCommandCount = Int(MIR4DDocumentCommandCount(engineDocument))
        engineRevision = MIR4DDocumentRevision(engineDocument)
        engineIsValid = MIR4DDocumentIsValid(engineDocument)
        engineIsModified = MIR4DDocumentIsModified(engineDocument)
    }
#endif

    private func publishChange() {
        objectWillChange.send()
        NotificationCenter.default.post(name: .mir4DModelChanged, object: self)
    }
}
