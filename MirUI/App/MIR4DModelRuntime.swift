import Foundation
import Combine
import SwiftUI

extension Notification.Name {
    static let mir4DModelChanged = Notification.Name("MIR4D.ModelChanged")
}

/// Live document model used by the SwiftUI workspace.
/// MirEngine remains the authority for evaluated B-Rep/topology; this object
/// stores the parametric document contract needed by the UI and persistence layer.
@MainActor
final class MIR4DModelRuntime: ObservableObject {
    static let shared = MIR4DModelRuntime()

    @Published private(set) var document: MIR4DModelDocument
    @Published private(set) var revision: UInt64 = 0
    private var createBoxObserver: NSObjectProtocol?

    private init() {
        document = MIR4DModelDocument.newProject(name: "Новый проект")
        createBoxObserver = NotificationCenter.default.addObserver(forName: .mir4DCreateBox, object: nil, queue: .main) { [weak self] notification in
            guard let payload = notification.object as? [String: Any], let width = payload["width"] as? Double, let depth = payload["depth"] as? Double, let height = payload["height"] as? Double else { return }
            Task { @MainActor [weak self] in self?.addBox(width: width, depth: depth, height: height) }
        }
    }

    deinit {
        if let observer = createBoxObserver { NotificationCenter.default.removeObserver(observer) }
    }

    func reset(projectName: String) {
        document = MIR4DModelDocument.newProject(name: projectName)
        revision &+= 1
        publishChange()
    }

    func load(_ document: MIR4DModelDocument) {
        self.document = document
        revision &+= 1
        publishChange()
    }

    @discardableResult
    func addBox(width: Double, depth: Double, height: Double, engineObjectID: UInt64? = nil) -> UUID {
        let bodyID = UUID()
        let operationID = UUID()
        let geometryID = UUID()
        let body = MIR4DBody(id: bodyID, name: "Тело \(document.bodies.count + 1)")
        let operation = MIR4DOperation(id: operationID, name: "Параллелепипед \(document.operations.count + 1)", kind: .extrude, bodyID: bodyID, parameters: ["width": width, "depth": depth, "height": height], featureIDs: [geometryID])
        var parameters: [String: Double] = ["width": width, "depth": depth, "height": height]
        if let engineObjectID { parameters["engineObjectID"] = Double(engineObjectID) }
        let geometry = MIR4DGeometry(id: geometryID, kind: .box, operationID: operationID, parameters: parameters, source: "MirEngine")
        let operationNode = MIR4DModelNode(id: operationID, title: operation.name, kind: .operation, children: [MIR4DModelNode(id: geometryID, title: "Геометрия", kind: .result)])
        let bodyNode = MIR4DModelNode(id: bodyID, title: body.name, kind: .body, children: [operationNode])
        document.bodies.append(body)
        document.operations.append(operation)
        document.geometry.append(geometry)
        document.root.children.append(bodyNode)
        revision &+= 1
        publishChange()
        return bodyID
    }

    private func publishChange() {
        NotificationCenter.default.post(name: .mir4DModelChanged, object: revision)
    }
}
