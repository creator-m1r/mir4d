import Foundation

/// Модели подсистемы совместной работы инженеров над одним проектом.
///
/// Все типы `Sendable`/`Codable`, чтобы безопасно передаваться между
/// сетевым `actor`-транспортом и главным потоком UI и сериализоваться
/// в Wire-формат (JSON) для WebSocket-потока.
public enum MirCollaborationModels {}

// MARK: - Lamport-часы упорядочивания операций

/// Логические часы Лампорта для детерминированного упорядочивания операций
/// при одновременном редактировании одного проекта несколькими инженерами.
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

// MARK: - Участник совместной работы (presence)

/// Право доступа участника к общему проекту.
public enum MirCollaboratorPermission: String, Codable, Sendable, Equatable {
    case editor
    case viewer
}

/// Нормализованная позиция курсора/выделения участника в视口е (0…1).
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
    /// Цвет присутствия в формате hex (#RRGGBB) для выделения выбора/курсора.
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

// MARK: - Версионирование общего проекта

/// Тег версии опубликованного общего проекта (для автосохранения/веток).
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

// MARK: - Операция над проектом

public struct MirCollaborationOperation: Codable, Identifiable, Sendable, Equatable {
    public enum OperationKind: String, Codable, Sendable, Equatable {
        case create
        case transform
        case delete
        case rename
        case updateMeta
        /// Только индикация выбора/курсора, не меняет геометрию.
        case select
    }

    public var id: String
    public var clock: MirOperationClock
    public var entityID: String
    public var kind: OperationKind
    /// Параметры операции в JSON (позиция, размеры, матрица преобразования и т.п.).
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

// MARK: - Снимок состояния проекта (начальная синхронизация)

public struct MirCollaborationEntityState: Codable, Sendable, Equatable {
    public var entityID: String
    public var type: String
    /// Матрица преобразования 4x4 (16 значений) для позиции/ориентации.
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

// MARK: - Конфликт

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

// MARK: - Wire-конверт для WebSocket-потока

/// Сообщение совместной работы в транспортном потоке `/api/team/stream`.
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

// MARK: - CRDT состояния сущностей (LWW-Register)

/// Состояние сущности как LWW-регистр: побеждает операция с большим часам
/// Лампорта (при равенстве — по authorID). Слияние коммутативно и идемпотентно.
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

    /// Объединить входящее состояние по правилу LWW.
    public func merged(with incoming: MirEntityState) -> MirEntityState {
        incoming.clock >= clock ? incoming : self
    }
}

/// Конвергентный CRDT множества сущностей общего проекта.
///
/// Гарантирует, что после обмена всеми операциями все участники получают
/// идентичное состояние, независимо от порядка доставки (в отличие от
/// наивного last-writer-wins, который не был коммутативным).
public struct MirCollaborationCRDT: Codable, Sendable {
    private var entities: [String: MirEntityState] = [:]

    public init() {}

    /// Применить операцию к CRDT (возвращает true, если состояние сущности изменилось).
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

    /// Слить состояние другого узла (коммутативно).
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

// MARK: - Параметры операций

/// Параметры операции создания примитива (параллелепипеда).
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

/// Параметры преобразования сущности. Матрица 4x4, построчно (16 значений).
public struct MirCollaborationTransform: Codable, Sendable {
    public static let identity: [Double] = [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]

    public var matrix: [Double]

    public init(matrix: [Double] = MirCollaborationTransform.identity) {
        self.matrix = matrix.count == 16 ? matrix : MirCollaborationTransform.identity
    }
}

/// Параметры переименования сущности.
public struct MirCollaborationRenameParameters: Codable, Sendable {
    public var name: String

    public init(name: String) {
        self.name = name
    }
}
