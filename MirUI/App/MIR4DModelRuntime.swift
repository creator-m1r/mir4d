import Foundation
import Combine
import SwiftUI

extension Notification.Name {
    static let mir4DModelChanged = Notification.Name("MIR4D.ModelChanged")
    static let mir4DEngineDocumentChanged = Notification.Name("MIR4D.EngineDocumentChanged")
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
    private var engineDocument: UnsafeMutableRawPointer?
#endif

    private init() {
        document = MIR4DModelDocument.newProject(name: "Новый проект")
#if !MIR4D_SWIFTPM
        engineDocument = MIR4DDocumentCreate()
        syncEngineState()
#endif
    }

    deinit {
#if !MIR4D_SWIFTPM
        if let engineDocument { MIR4DDocumentDestroy(engineDocument) }
#endif
    }

    func reset(projectName: String) {
        document = MIR4DModelDocument.newProject(name: projectName)
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
#if !MIR4D_SWIFTPM
        if engineDocument == nil { engineDocument = MIR4DDocumentCreate() }
        document.name.withCString { MIR4DDocumentReset(engineDocument, $0) }

        // Rebuild the evaluated MirEngine document from the persisted
        // parameter model. The renderer receives the same geometry below.
        for geometry in document.geometry where geometry.kind == .box {
            let width = geometry.parameters["width"] ?? 0
            let depth = geometry.parameters["depth"] ?? 0
            let height = geometry.parameters["height"] ?? 0
            guard width > 0, depth > 0, height > 0 else { continue }
            var objectID: UInt64 = 0
            _ = MIR4DDocumentCreateBox(engineDocument, width, depth, height, &objectID)
        }
        syncEngineState()
#endif
        revision &+= 1
        publishChange()
        replayGeometryToViewport()
    }

    @discardableResult
    func addBox(width: Double, depth: Double, height: Double, engineObjectID: UInt64? = nil) -> UUID {
        var resolvedEngineObjectID = engineObjectID
#if !MIR4D_SWIFTPM
        if resolvedEngineObjectID == nil {
            var objectID: UInt64 = 0
            guard MIR4DDocumentCreateBox(engineDocument, width, depth, height, &objectID) else { return UUID() }
            resolvedEngineObjectID = objectID
        }
#endif
        let bodyID = UUID()
        let operationID = UUID()
        let geometryID = UUID()
        let body = MIR4DBody(id: bodyID, name: "Тело \(document.bodies.count + 1)")
        let operation = MIR4DOperation(id: operationID, name: "Параллелепипед \(document.operations.count + 1)", kind: .extrude, bodyID: bodyID, parameters: ["width": width, "depth": depth, "height": height], featureIDs: [geometryID])
        var parameters: [String: Double] = ["width": width, "depth": depth, "height": height]
        if let resolvedEngineObjectID { parameters["engineObjectID"] = Double(resolvedEngineObjectID) }
        let geometry = MIR4DGeometry(id: geometryID, kind: .box, operationID: operationID, parameters: parameters, source: "MirEngine")
        let operationNode = MIR4DModelNode(id: operationID, title: operation.name, kind: .operation, children: [MIR4DModelNode(id: geometryID, title: "Геометрия", kind: .result)])
        let bodyNode = MIR4DModelNode(id: bodyID, title: body.name, kind: .body, children: [operationNode])
        document.bodies.append(body)
        document.operations.append(operation)
        document.geometry.append(geometry)
        document.root.children.append(bodyNode)
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
                  width > 0, depth > 0, height > 0 else { return nil }
            return ["width": width, "depth": depth, "height": height]
        }

        guard !payloads.isEmpty else { return }

        // CADMainView creates the native OpenGL view immediately after the
        // project is loaded. Replay on the next run-loop turn so the renderer
        // can consume the persisted geometry during workspace hydration.
        DispatchQueue.main.async { [weak self] in
            guard self != nil else { return }
            for payload in payloads {
                NotificationCenter.default.post(name: .mir4DCreateBox, object: payload)
            }
        }
    }

    private func publishChange() {
        NotificationCenter.default.post(name: .mir4DModelChanged, object: revision)
    }

#if !MIR4D_SWIFTPM
    private func syncEngineState() {
        engineObjectCount = MIR4DDocumentObjectCount(engineDocument)
        engineCommandCount = MIR4DDocumentCommandCount(engineDocument)
        engineRevision = MIR4DDocumentRevision(engineDocument)
        engineIsModified = MIR4DDocumentIsModified(engineDocument)
        engineIsValid = MIR4DDocumentIsValid(engineDocument)
        NotificationCenter.default.post(name: .mir4DEngineDocumentChanged, object: engineRevision)
    }
#endif
}
