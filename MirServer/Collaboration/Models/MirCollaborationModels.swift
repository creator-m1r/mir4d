import Foundation

public enum MirCollaborationModels {}

public struct MirOperationClock: Codable, Sendable, Equatable, Comparable {
    public var counter: UInt64
    public var authorID: String

    public init(counter: UInt64 = 0, authorID: String = "") {
        self.counter = counter
        self.authorID = authorID
    }

    public static func < (lhs: MirOperationClock, rhs: MirOperationClock) -> Bool {
        if lhs.counter != rhs.counter { return lhs.counter < rhs.counter }
        return lhs.authorID < rhs.authorID
    }
}

public enum MirCollaboratorPermission: String, Codable, Sendable, Equatable {
    case editor
    case viewer
}

public struct MirCollaboratorCursor: Codable, Sendable, Equatable {
    public var x: Double
    public var y: Double
    public var selectedEntityID: String?

    public init(x: Double, y: Double, selectedEntityID: String? = nil) {
        self.x = x
        self.y = y
        self.selectedEntityID = selectedEntityID
    }
}

public struct MirCollaborator: Codable, Identifiable, Sendable, Equatable {
    public var id: String
    public var displayName: String
    public var role: String
    public var permission: MirCollaboratorPermission

    public var color: String
    public var lastSeen: Date
    public var active: Bool
    public var cursor: MirCollaboratorCursor?

    public init(
        id: String,
        displayName: String,
        role: String,
        color: String,
        permission: MirCollaboratorPermission = .editor,
        lastSeen: Date = Date(),
        active: Bool = true,
        cursor: MirCollaboratorCursor? = nil
    ) {
        self.id = id
        self.displayName = displayName
        self.role = role
        self.permission = permission
        self.color = color
        self.lastSeen = lastSeen
        self.active = active
        self.cursor = cursor
    }
}

public struct MirProjectVersion: Codable, Sendable, Equatable, Identifiable {
    public var id: String
    public var label: String
    public var number: UInt64
    public var authorID: String
    public var at: Date

    public init(label: String, number: UInt64, authorID: String, at: Date = Date()) {
        self.id = UUID().uuidString
        self.label = label
        self.number = number
        self.authorID = authorID
        self.at = at
    }
}

public struct MirCollaborationOperation: Codable, Identifiable, Sendable, Equatable {
    public enum OperationKind: String, Codable, Sendable, Equatable {
        case create
        case transform
        case delete
        case rename
        case updateMeta

        case select
    }

    public var id: String
    public var clock: MirOperationClock
    public var entityID: String
    public var kind: OperationKind

    public var parameters: Data
    public var authorID: String
    public var timestamp: Date

    public init(
        id: String = UUID().uuidString,
        clock: MirOperationClock,
        entityID: String,
        kind: OperationKind,
        parameters: Data = Data(),
        authorID: String,
        timestamp: Date = Date()
    ) {
        self.id = id
        self.clock = clock
        self.entityID = entityID
        self.kind = kind
        self.parameters = parameters
        self.authorID = authorID
        self.timestamp = timestamp
    }
}

public struct MirCollaborationEntityState: Codable, Sendable, Equatable {
    public var entityID: String
    public var type: String

    public var transform: [Double]
    public var name: String

    public init(entityID: String, type: String, transform: [Double], name: String) {
        self.entityID = entityID
        self.type = type
        self.transform = transform
        self.name = name
    }
}

public struct MirProjectSnapshot: Codable, Sendable, Equatable {
    public var projectID: String
    public var entities: [MirCollaborationEntityState]
    public var baseClock: MirOperationClock

    public init(projectID: String, entities: [MirCollaborationEntityState], baseClock: MirOperationClock) {
        self.projectID = projectID
        self.entities = entities
        self.baseClock = baseClock
    }
}

public struct MirConflict: Codable, Identifiable, Sendable, Equatable {
    public var id: String
    public var entityID: String
    public var winnerAuthorID: String
    public var loserAuthorID: String
    public var resolvedAt: Date

    public init(entityID: String, winnerAuthorID: String, loserAuthorID: String, resolvedAt: Date = Date()) {
        self.id = UUID().uuidString
        self.entityID = entityID
        self.winnerAuthorID = winnerAuthorID
        self.loserAuthorID = loserAuthorID
        self.resolvedAt = resolvedAt
    }
}

public enum MirCollaborationWireMessage: Codable, Sendable {
    case operation(MirCollaborationOperation)
    case presence(MirCollaborator)
    case snapshot(MirProjectSnapshot)
    case requestSnapshot
}

public struct MirCollaborationEnvelope: Codable, Sendable {
    public var projectID: String
    public var message: MirCollaborationWireMessage

    public init(projectID: String, message: MirCollaborationWireMessage) {
        self.projectID = projectID
        self.message = message
    }
}

public struct MirEntityState: Codable, Sendable, Equatable {
    public var transform: [Double]
    public var name: String
    public var deleted: Bool
    public var clock: MirOperationClock

    public init(transform: [Double], name: String, deleted: Bool, clock: MirOperationClock) {
        self.transform = transform
        self.name = name
        self.deleted = deleted
        self.clock = clock
    }

    public func merged(with incoming: MirEntityState) -> MirEntityState {
        incoming.clock >= clock ? incoming : self
    }
}

public struct MirCollaborationCRDT: Codable, Sendable {
    private var entities: [String: MirEntityState] = [:]

    public init() {}

    @discardableResult
    public mutating func apply(_ op: MirCollaborationOperation) -> Bool {
        let prev = entities[op.entityID]
        let incoming: MirEntityState
        switch op.kind {
        case .delete:
            incoming = MirEntityState(transform: prev?.transform ?? [], name: prev?.name ?? "", deleted: true, clock: op.clock)
        case .transform:
            let matrix = (try? JSONDecoder().decode([Double].self, from: op.parameters)) ?? prev?.transform ?? []
            incoming = MirEntityState(transform: matrix, name: prev?.name ?? "", deleted: prev?.deleted ?? false, clock: op.clock)
        case .rename, .updateMeta:
            let name = (try? JSONDecoder().decode([String: String].self, from: op.parameters))?["name"] ?? prev?.name ?? ""
            incoming = MirEntityState(transform: prev?.transform ?? [], name: name, deleted: prev?.deleted ?? false, clock: op.clock)
        case .create, .select:
            incoming = MirEntityState(transform: prev?.transform ?? [], name: prev?.name ?? "", deleted: false, clock: op.clock)
        }

        guard let previous = prev else {
            entities[op.entityID] = incoming
            return true
        }
        let merged = previous.merged(with: incoming)
        entities[op.entityID] = merged
        return merged != previous
    }

    public mutating func merge(_ other: MirCollaborationCRDT) {
        for (id, state) in other.entities {
            if let local = entities[id] {
                entities[id] = local.merged(with: state)
            } else {
                entities[id] = state
            }
        }
    }

    public func state(for entityID: String) -> MirEntityState? {
        entities[entityID]
    }

    public var allEntityIDs: [String] { Array(entities.keys) }
}

public struct MirCollaborationCreateParameters: Codable, Sendable {
    public var width: Double
    public var depth: Double
    public var height: Double

    public init(width: Double = 1, depth: Double = 1, height: Double = 1) {
        self.width = width
        self.depth = depth
        self.height = height
    }
}

public struct MirCollaborationTransform: Codable, Sendable {
    public static let identity: [Double] = [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]

    public var matrix: [Double]

    public init(matrix: [Double] = MirCollaborationTransform.identity) {
        self.matrix = matrix.count == 16 ? matrix : MirCollaborationTransform.identity
    }
}

public struct MirCollaborationRenameParameters: Codable, Sendable {
    public var name: String

    public init(name: String) {
        self.name = name
    }
}
