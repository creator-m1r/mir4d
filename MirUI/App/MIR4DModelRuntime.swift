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

    nonisolated(unsafe) private var engineDocument: UnsafeMutableRawPointer?
#endif

    nonisolated(unsafe) var viewport: UnsafeMutableRawPointer?

    nonisolated(unsafe) var renderer: UnsafeMutableRawPointer?

    nonisolated(unsafe) var glContext: UnsafeMutableRawPointer?

    nonisolated(unsafe) var activeSketchPlane: SketchPlaneAnchor?

    private let handGrabController = MIRHandGrabController()

    private var cancellables = Set<AnyCancellable>()

    private var handSkeletonTopologySent = false
    private var lastSkeletonStyleSignature: String?
    private var lastSkeletonDataSignature: String?

    private var engineObjectIDs: [UUID: UInt64] = [:]

    private var viewportObjectIDs: [UUID: UInt64] = [:]

    func registerViewportEngineID(bodyID: UUID, engineObjectID: UInt64) {
        guard engineObjectID > 0 else { return }
        viewportObjectIDs[bodyID] = engineObjectID
    }

    @discardableResult
    func deformSelected(x: Double, y: Double, z: Double, radius: Double, strength: Double, mode: Int) -> Bool {
        guard let viewport else { return false }
        return mirEngineDeformSelected(viewport, x, y, z, radius, strength, Int32(mode))
    }

    func beginDeformStroke() {
        guard let viewport else { return }
        _ = mirEngineBeginDeformSelected(viewport)
    }

    func endDeformStroke() {
        guard let viewport else { return }
        _ = mirEngineEndDeformSelected(viewport)
    }

    func persistedBodyID(forViewportEngineObjectID objectID: UInt64) -> UUID? {
        guard objectID > 0 else { return nil }
        for (bodyID, viewportID) in viewportObjectIDs where viewportID == objectID {
            return bodyID
        }
        return nil
    }

    func selectBody(persistedID: UUID) {
        guard let viewport else { return }
        guard let objectID = viewportObjectIDs[persistedID], objectID > 0 else { return }
        mirEngineSelectObject(viewport, objectID)
    }

    func pickWorldPoint(nx: Double, ny: Double) -> (point: SIMD3<Double>, objectId: UInt64)? {
        guard let viewport else { return nil }
        var x = 0.0, y = 0.0, z = 0.0
        var objectId: UInt64 = 0
        guard mirEnginePickWorldPoint(viewport, nx, ny, &x, &y, &z, &objectId) else { return nil }
        return (SIMD3(x, y, z), objectId)
    }

    func cameraEye() -> SIMD3<Double>? {
        guard let viewport else { return nil }
        var x = 0.0, y = 0.0, z = 0.0
        guard MirEngineGetCameraEye(viewport, &x, &y, &z) else { return nil }
        return SIMD3(x, y, z)
    }

    func pickHandRay(origin: SIMD3<Double>, direction: SIMD3<Double>) -> (objectId: UInt64, distance: Double)? {
        guard let viewport else { return nil }
        var objectId: UInt64 = 0
        var distance = 0.0
        guard MirEnginePickHandRay(viewport, origin.x, origin.y, origin.z,
                                   direction.x, direction.y, direction.z,
                                   &objectId, &distance) else { return nil }
        return (objectId, distance)
    }

    func beginGrab(objectId: UInt64) {
        guard let viewport, objectId > 0 else { return }
        MirEngineBeginGrab(viewport, objectId)
    }

    func previewGrab(objectId: UInt64, transform: MirTransform) {
        guard let viewport, objectId > 0 else { return }
        _ = MirEnginePreviewGrab(viewport, objectId, transform)
    }

    @discardableResult
    func commitGrab(objectId: UInt64) -> Bool {
        guard let viewport, objectId > 0 else { return false }
        return MirEngineCommitGrab(viewport, objectId)
    }

    func cancelGrab() {
        guard let viewport else { return }
        MirEngineCancelGrab(viewport)
    }

    func getObjectTransform(objectId: UInt64) -> MirTransform? {
        guard let viewport, objectId > 0 else { return nil }
        var t = MirTransform()
        guard MirEngineGetObjectTransform(viewport, objectId, &t) else { return nil }
        return t
    }

    func setHandHover(objectId: UInt64) {
        guard let viewport, objectId > 0 else { return }
        MirEngineSetHandHover(viewport, objectId)
    }

    func setHandSkeleton(frames: [MIRHandSkeletonFrame]) {
        guard let viewport else { return }
        let mode = MIRHandGestureModule.shared.configuration.skeletonVisualizationMode
        guard mode != .off, !frames.isEmpty else {
            clearHandSkeleton()
            return
        }

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

    func clearHandSkeleton() {
        guard let viewport else { return }
        MirEngineClearHandSkeleton(viewport)
    }

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

    func renameBody(_ bodyID: UUID, to name: String) {
        guard let index = document.bodies.firstIndex(where: { $0.id == bodyID }) else { return }
        document.bodies[index].name = name
        if let nodeIndex = document.root.children.firstIndex(where: { $0.id == bodyID }) {
            document.root.children[nodeIndex].title = name
        }
        revision &+= 1
        publishChange()
    }

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
