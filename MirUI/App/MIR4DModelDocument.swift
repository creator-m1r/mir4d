import Foundation

/// Persistent CAD model layer shared by the SwiftUI document and the future MirEngine bridge.
/// Geometry is represented by stable IDs and parametric descriptors; the C++ kernel remains
/// the authority for evaluated topology and B-Rep data.
struct MIR4DModelDocument: Codable, Equatable {
    var schemaVersion: Int = 1
    var root: MIR4DModelNode
    var bodies: [MIR4DBody] = []
    var operations: [MIR4DOperation] = []
    var geometry: [MIR4DGeometry] = []

    static func newProject(name: String) -> MIR4DModelDocument {
        let bodyID = UUID()
        return MIR4DModelDocument(
            root: MIR4DModelNode(id: UUID(), title: name, kind: .project, children: [
                MIR4DModelNode(id: bodyID, title: "Тело", kind: .body, children: [])
            ]),
            bodies: [MIR4DBody(id: bodyID, name: "Тело")]
        )
    }

    // MARK: - Stable identity lookup

    /// Returns the persisted body for a stable model UUID.
    func body(id: UUID) -> MIR4DBody? {
        bodies.first { $0.id == id }
    }

    /// Returns the persisted operation for a stable model UUID.
    func operation(id: UUID) -> MIR4DOperation? {
        operations.first { $0.id == id }
    }

    /// Returns the persisted geometry descriptor for a stable model UUID.
    func geometry(id: UUID) -> MIR4DGeometry? {
        geometry.first { $0.id == id }
    }

    /// Resolves the owning body of an operation without requiring the UI tree.
    func bodyID(forOperation id: UUID) -> UUID? {
        operation(id: id)?.bodyID
    }

    /// Returns all feature/geometry IDs belonging to a body through operations.
    func geometryIDs(forBody bodyID: UUID) -> [UUID] {
        let operationIDs = operations
            .filter { $0.bodyID == bodyID }
            .map(\.id)
        return geometry
            .filter { geometry in
                guard let operationID = geometry.operationID else { return false }
                return operationIDs.contains(operationID)
            }
            .map(\.id)
    }

    /// Resolves a persisted body UUID from the engine object identity stored on a geometry.
    /// This is deliberately a model-layer lookup: UI selection must not invent a second ID.
    func bodyID(forEngineObjectID objectID: UInt64) -> UUID? {
        guard objectID > 0 else { return nil }
        for geometry in geometry {
            guard let raw = geometry.parameters["engineObjectID"], raw >= 0 else { continue }
            guard raw.rounded() == raw, raw <= Double(UInt64.max) else { continue }
            guard UInt64(raw) == objectID else { continue }
            guard let operationID = geometry.operationID else { continue }
            if let bodyID = bodyID(forOperation: operationID) {
                return bodyID
            }
        }
        return nil
    }
}

struct MIR4DModelNode: Codable, Equatable, Identifiable {
    enum Kind: String, Codable { case project, component, body, sketch, operation, result }
    var id: UUID
    var title: String
    var kind: Kind
    var children: [MIR4DModelNode] = []
}

struct MIR4DBody: Codable, Equatable, Identifiable {
    var id: UUID
    var name: String
    var visible: Bool = true
    var suppressed: Bool = false
}

struct MIR4DOperation: Codable, Equatable, Identifiable {
    enum Kind: String, Codable { case sketch, extrude, revolve, fillet, chamfer, boolean, transform, imported }
    var id: UUID
    var name: String
    var kind: Kind
    var bodyID: UUID
    var parameters: [String: Double] = [:]
    var featureIDs: [UUID] = []
    var suppressed: Bool = false
}

struct MIR4DGeometry: Codable, Equatable, Identifiable {
    enum Kind: String, Codable { case point, line, circle, plane, box, cylinder, mesh, brep }
    var id: UUID
    var kind: Kind
    var operationID: UUID?
    var parameters: [String: Double] = [:]
    var source: String = "MirEngine"
}

extension MIR4DModelDocument {
    static func from(tree: [TreeNodeData], projectName: String) -> MIR4DModelDocument {
        func convert(_ node: TreeNodeData, parentKind: MIR4DModelNode.Kind? = nil) -> MIR4DModelNode {
            let kind: MIR4DModelNode.Kind
            let lower = node.name.lowercased()
            if lower.contains("sketch") { kind = .sketch }
            else if lower.contains("extrude") || lower.contains("fillet") || lower.contains("операц") { kind = .operation }
            else if lower.contains("тело") || lower.contains("body") { kind = .body }
            else if lower.contains("сбор") || lower.contains("component") { kind = .component }
            else { kind = parentKind == nil ? .project : .component }
            return MIR4DModelNode(id: node.id, title: node.name, kind: kind, children: node.children.map { convert($0, parentKind: kind) })
        }
        return MIR4DModelDocument(root: MIR4DModelNode(id: UUID(), title: projectName, kind: .project, children: tree.flatMap { $0.children.isEmpty ? [convert($0)] : $0.children.map { convert($0, parentKind: .project) } }))
    }
}
