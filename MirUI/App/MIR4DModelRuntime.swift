import Foundation
import Combine
import SwiftUI

/// Live document model used by the SwiftUI workspace.
/// MirEngine remains the authority for evaluated B-Rep/topology; this object
/// stores the parametric document contract needed by the UI and persistence layer.
@MainActor
final class MIR4DModelRuntime: ObservableObject {
    static let shared = MIR4DModelRuntime()

    @Published private(set) var document: MIR4DModelDocument
    @Published private(set) var revision: UInt64 = 0

    private init() {
        document = MIR4DModelDocument.newProject(name: "Новый проект")
    }

    func reset(projectName: String) {
        document = MIR4DModelDocument.newProject(name: projectName)
        revision &+= 1
    }

    func load(_ document: MIR4DModelDocument) {
        self.document = document
        revision &+= 1
    }

    @discardableResult
    func addBox(width: Double, depth: Double, height: Double, engineObjectID: UInt64? = nil) -> UUID {
        let bodyID = UUID()
        let operationID = UUID()
        let geometryID = UUID()

        let body = MIR4DBody(id: bodyID, name: "Тело \(document.bodies.count + 1)")
        let operation = MIR4DOperation(
            id: operationID,
            name: "Параллелепипед \(document.operations.count + 1)",
            kind: .extrude,
            bodyID: bodyID,
            parameters: [
                "width": width,
                "depth": depth,
                "height": height
            ],
            featureIDs: [geometryID]
        )
        var geometryParameters: [String: Double] = [
            "width": width,
            "depth": depth,
            "height": height
        ]
        if let engineObjectID {
            geometryParameters["engineObjectID"] = Double(engineObjectID)
        }
        let geometry = MIR4DGeometry(
            id: geometryID,
            kind: .box,
            operationID: operationID,
            parameters: geometryParameters,
            source: "MirEngine"
        )

        let operationNode = MIR4DModelNode(
            id: operationID,
            title: operation.name,
            kind: .operation,
            children: [
                MIR4DModelNode(id: geometryID, title: "Геометрия", kind: .result)
            ]
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
        revision &+= 1
        return bodyID
    }
}
