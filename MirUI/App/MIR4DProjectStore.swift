import Foundation

struct MIR4DProjectManifest: Codable {
    var formatVersion: Int = 1
    var name: String
    var createdAt: Date
    var modifiedAt: Date
    var workbench: String
    var subMode: String
    var selectedTreeItem: String
    var gridVisible: Bool
    var axesVisible: Bool
    var sectionMode: Bool
    var currentTime: Double
}

@MainActor
final class MIR4DProjectStore {
    static let shared = MIR4DProjectStore()

    private let fileManager = FileManager.default
    private let manifestFileName = "project.mir4d.json"
    private let modelFileName = "Scenes/model.mir4d.json"

    private init() {}

    func createProject(name: String, in parentURL: URL) throws -> URL {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { throw ProjectError.invalidName }
        let safeName = sanitizedName(trimmed)
        let projectURL = parentURL.appendingPathComponent("\(safeName).mir4d", isDirectory: true)
        try fileManager.createDirectory(at: projectURL, withIntermediateDirectories: true)
        for folder in ["Models", "Scenes", "Results", "Documents"] {
            try fileManager.createDirectory(at: projectURL.appendingPathComponent(folder, isDirectory: true), withIntermediateDirectories: true)
        }
        return projectURL
    }

    func save(manifest: MIR4DProjectManifest, to projectURL: URL) throws {
        let encoder = makeEncoder()
        let data = try encoder.encode(manifest)
        try data.write(to: projectURL.appendingPathComponent(manifestFileName), options: .atomic)
    }

    func load(from projectURL: URL) throws -> MIR4DProjectManifest {
        let data = try Data(contentsOf: projectURL.appendingPathComponent(manifestFileName))
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        return try decoder.decode(MIR4DProjectManifest.self, from: data)
    }

    /// Synchronous save used by explicit user commands such as Cmd-S.
    func saveModel(_ model: MIR4DModelDocument, to projectURL: URL) throws {
        let data = try makeEncoder().encode(model)
        try writeModelData(data, to: projectURL)
    }

    /// Encodes the value away from the main actor and performs the atomic file write there as well.
    /// The model is a value snapshot: after this method is called, the runtime may continue editing its live document.
    func saveModelAsync(_ model: MIR4DModelDocument, to projectURL: URL) async throws {
        let snapshot = MIR4DModelSnapshot(model: model)
        let data = try await Task.detached(priority: .utility) {
            try Self.makeEncoder().encode(snapshot.model)
        }.value

        try await Task.detached(priority: .utility) {
            try Self.writeModelData(data, to: projectURL)
        }.value
    }

    func loadModel(from projectURL: URL) throws -> MIR4DModelDocument {
        let url = projectURL.appendingPathComponent(modelFileName)
        let data = try Data(contentsOf: url)
        return try JSONDecoder().decode(MIR4DModelDocument.self, from: data)
    }

    private static func makeEncoder() -> JSONEncoder {
        let encoder = JSONEncoder()
        // Compact JSON is used for production saves/autosave. Pretty JSON is better suited to exports/debugging.
        encoder.outputFormatting = [.sortedKeys, .withoutEscapingSlashes]
        encoder.dateEncodingStrategy = .iso8601
        return encoder
    }

    private func makeEncoder() -> JSONEncoder { Self.makeEncoder() }

    private static func writeModelData(_ data: Data, to projectURL: URL) throws {
        let fileManager = FileManager.default
        let url = projectURL.appendingPathComponent("Scenes/model.mir4d.json")
        try fileManager.createDirectory(
            at: url.deletingLastPathComponent(),
            withIntermediateDirectories: true
        )
        // Keep atomic replacement: a killed process must never leave a half-written CAD model.
        try data.write(to: url, options: .atomic)
    }

    private func writeModelData(_ data: Data, to projectURL: URL) throws {
        try Self.writeModelData(data, to: projectURL)
    }

    private func sanitizedName(_ name: String) -> String {
        let forbidden = CharacterSet(charactersIn: "/:\\")
        let result = name.components(separatedBy: forbidden).joined(separator: "-").trimmingCharacters(in: .whitespacesAndNewlines)
        return result.isEmpty ? "Новый проект" : result
    }

    enum ProjectError: LocalizedError {
        case invalidName
        var errorDescription: String? { "Введите название проекта." }
    }
}

/// Explicit value snapshot used to move model encoding to a utility task.
/// MIR4DModelDocument is a value model; no live runtime reference is retained by the save operation.
private struct MIR4DModelSnapshot: @unchecked Sendable {
    let model: MIR4DModelDocument
}
