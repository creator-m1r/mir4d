import Foundation

/// Модели данных подсистемы MirServer.
///
/// Все типы являются `Sendable`, чтобы безопасно передаваться между
/// изолированным сетевым `actor`-транспортом и главным потоком UI
/// через Event Bus (NotificationCenter).
public enum MirServerModels {}

// MARK: - Участники команды

public struct MirTeamMember: Codable, Identifiable, Sendable, Equatable, Hashable {
    public var id: String
    public var displayName: String
    public var role: String
    public var online: Bool

    public init(id: String, displayName: String, role: String, online: Bool = false) {
        self.id = id
        self.displayName = displayName
        self.role = role
        self.online = online
    }
}

// MARK: - Сообщения команды

public struct MirTeamMessage: Codable, Identifiable, Sendable, Equatable, Hashable {
    public var id: String
    public var authorID: String
    public var authorName: String
    public var text: String
    public var timestamp: Date
    public var projectID: String?

    public init(
        id: String = UUID().uuidString,
        authorID: String,
        authorName: String,
        text: String,
        timestamp: Date = Date(),
        projectID: String? = nil
    ) {
        self.id = id
        self.authorID = authorID
        self.authorName = authorName
        self.text = text
        self.timestamp = timestamp
        self.projectID = projectID
    }
}

// MARK: - Экспорт проекта

public struct MirProjectExportRequest: Codable, Sendable, Equatable {
    public var projectID: String
    public var projectName: String
    public var format: String
    public var payload: Data
    public var message: String?

    public init(
        projectID: String,
        projectName: String,
        format: String,
        payload: Data,
        message: String? = nil
    ) {
        self.projectID = projectID
        self.projectName = projectName
        self.format = format
        self.payload = payload
        self.message = message
    }
}

public struct MirProjectExportResult: Codable, Sendable, Equatable {
    public var projectID: String
    public var remoteURL: String
    public var exportedAt: Date
    public var bytes: Int

    public init(projectID: String, remoteURL: String, exportedAt: Date, bytes: Int) {
        self.projectID = projectID
        self.remoteURL = remoteURL
        self.exportedAt = exportedAt
        self.bytes = bytes
    }
}

// MARK: - Статус соединения

public enum MirServerConnectionStatus: Sendable, Equatable {
    case disconnected
    case connecting
    case connected
    case failed(String)

    /// Локализованная метка статуса для UI.
    public var label: String {
        switch self {
        case .disconnected: return "Не подключено"
        case .connecting: return "Подключение…"
        case .connected: return "Подключено"
        case .failed(let reason): return "Ошибка: \(reason)"
        }
    }
}
