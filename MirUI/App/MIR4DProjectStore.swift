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

    private init() {}

    func createProject(name: String, in parentURL: URL) throws -> URL {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else {
            throw ProjectError.invalidName
        }

        let safeName = sanitizedName(trimmed)
        let projectURL = parentURL.appendingPathComponent("\(safeName).mir4d", isDirectory: true)
        try fileManager.createDirectory(at: projectURL, withIntermediateDirectories: true)
        try fileManager.createDirectory(at: projectURL.appendingPathComponent("Models", isDirectory: true), withIntermediateDirectories: true)
        try fileManager.createDirectory(at: projectURL.appendingPathComponent("Scenes", isDirectory: true), withIntermediateDirectories: true)
        try fileManager.createDirectory(at: projectURL.appendingPathComponent("Results", isDirectory: true), withIntermediateDirectories: true)
        try fileManager.createDirectory(at: projectURL.appendingPathComponent("Documents", isDirectory: true), withIntermediateDirectories: true)

        return projectURL
    }

    func save(manifest: MIR4DProjectManifest, to projectURL: URL) throws {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys, .withoutEscapingSlashes]
        encoder.dateEncodingStrategy = .iso8601
        let data = try encoder.encode(manifest)
        try data.write(to: projectURL.appendingPathComponent(manifestFileName), options: .atomic)
    }

    func load(from projectURL: URL) throws -> MIR4DProjectManifest {
        let data = try Data(contentsOf: projectURL.appendingPathComponent(manifestFileName))
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        return try decoder.decode(MIR4DProjectManifest.self, from: data)
    }

    private func sanitizedName(_ name: String) -> String {
        let forbidden = CharacterSet(charactersIn: "/:\\")
        let components = name.components(separatedBy: forbidden)
        let result = components.joined(separator: "-").trimmingCharacters(in: .whitespacesAndNewlines)
        return result.isEmpty ? "Новый проект" : result
    }

    enum ProjectError: LocalizedError {
        case invalidName

        var errorDescription: String? {
            switch self {
            case .invalidName:
                return "Введите название проекта."
            }
        }
    }
}
