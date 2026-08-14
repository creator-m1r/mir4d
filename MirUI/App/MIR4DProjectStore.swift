import Foundation

private enum MIR4DPaths {
    static let modelRelativePath = "Scenes/model.mir4d.json"
}

struct MIR4DProjectManifest: Codable {
    static let currentFormat = "MIR4D"
    static let currentFormatVersion = 1

    var format: String = MIR4DProjectManifest.currentFormat
    var formatVersion: Int = MIR4DProjectManifest.currentFormatVersion
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
    var modelPath: String = MIR4DPaths.modelRelativePath

    // Reserved for the next manifest migration. Keeping the properties optional
    // lets version-1 packages remain readable while the richer identity contract is introduced.
    var appVersion: String? = nil
    var uuid: UUID? = nil
    var thumbnailPath: String? = "Thumbnails/preview.png"

    enum CodingKeys: String, CodingKey {
        case format, formatVersion, name, createdAt, modifiedAt
        case workbench, subMode, selectedTreeItem, gridVisible, axesVisible
        case sectionMode, currentTime, modelPath, appVersion, uuid, thumbnailPath
    }

    init(
        format: String = MIR4DProjectManifest.currentFormat,
        formatVersion: Int = MIR4DProjectManifest.currentFormatVersion,
        name: String,
        createdAt: Date,
        modifiedAt: Date,
        workbench: String,
        subMode: String,
        selectedTreeItem: String,
        gridVisible: Bool,
        axesVisible: Bool,
        sectionMode: Bool,
        currentTime: Double,
        modelPath: String = MIR4DPaths.modelRelativePath,
        appVersion: String? = nil,
        uuid: UUID? = nil,
        thumbnailPath: String? = "Thumbnails/preview.png"
    ) {
        self.format = format
        self.formatVersion = formatVersion
        self.name = name
        self.createdAt = createdAt
        self.modifiedAt = modifiedAt
        self.workbench = workbench
        self.subMode = subMode
        self.selectedTreeItem = selectedTreeItem
        self.gridVisible = gridVisible
        self.axesVisible = axesVisible
        self.sectionMode = sectionMode
        self.currentTime = currentTime
        self.modelPath = modelPath
        self.appVersion = appVersion
        self.uuid = uuid
        self.thumbnailPath = thumbnailPath
    }
}

@MainActor
final class MIR4DProjectStore {
    static let shared = MIR4DProjectStore()

    static let packageExtension = "mir4d"
    static let manifestFileName = "project.mir4d.json"
    static let modelRelativePath = MIR4DPaths.modelRelativePath

    private let fileManager = FileManager.default

    private init() {}

    func createProject(name: String, in parentURL: URL) throws -> URL {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { throw ProjectError.invalidName }

        let safeName = sanitizedName(trimmed)
        let projectURL = parentURL.appendingPathComponent(
            "\(safeName).\(Self.packageExtension)",
            isDirectory: true
        )

        guard !fileManager.fileExists(atPath: projectURL.path) else {
            throw ProjectError.alreadyExists
        }

        try fileManager.createDirectory(at: projectURL, withIntermediateDirectories: true)
        for folder in ["Models", "Scenes", "Results", "Documents", "Thumbnails"] {
            try fileManager.createDirectory(
                at: projectURL.appendingPathComponent(folder, isDirectory: true),
                withIntermediateDirectories: true
            )
        }
        return projectURL
    }

    func isValidPackage(at projectURL: URL) -> Bool {
        guard projectURL.pathExtension.lowercased() == Self.packageExtension else { return false }
        let manifestURL = projectURL.appendingPathComponent(Self.manifestFileName)
        let modelURL = projectURL.appendingPathComponent(Self.modelRelativePath)
        var isDirectory: ObjCBool = false
        guard fileManager.fileExists(atPath: projectURL.path, isDirectory: &isDirectory), isDirectory.boolValue else { return false }
        return fileManager.fileExists(atPath: manifestURL.path) && fileManager.fileExists(atPath: modelURL.path)
    }

    func save(manifest: MIR4DProjectManifest, to projectURL: URL) throws {
        try ensurePackageDirectory(projectURL)

        var normalized = manifest
        normalized.format = MIR4DProjectManifest.currentFormat
        normalized.formatVersion = MIR4DProjectManifest.currentFormatVersion
        normalized.modelPath = Self.modelRelativePath
        normalized.modifiedAt = Date()

        let encoder = makeEncoder()
        let data = try encoder.encode(normalized)
        try data.write(
            to: projectURL.appendingPathComponent(Self.manifestFileName),
            options: .atomic
        )
    }

    func load(from projectURL: URL) throws -> MIR4DProjectManifest {
        guard projectURL.pathExtension.lowercased() == Self.packageExtension else {
            throw ProjectError.invalidPackage
        }

        let manifestURL = projectURL.appendingPathComponent(Self.manifestFileName)
        guard fileManager.fileExists(atPath: manifestURL.path) else {
            throw ProjectError.invalidPackage
        }

        do {
            let data = try Data(contentsOf: manifestURL)
            let decoder = makeDecoder()
            let manifest = try decoder.decode(MIR4DProjectManifest.self, from: data)

            guard manifest.format == MIR4DProjectManifest.currentFormat else {
                throw ProjectError.unknownFormat(manifest.format)
            }
            guard manifest.formatVersion <= MIR4DProjectManifest.currentFormatVersion else {
                throw ProjectError.unsupportedVersion(manifest.formatVersion)
            }
            guard isSafeRelativePath(manifest.modelPath),
                  manifest.modelPath == Self.modelRelativePath else {
                throw ProjectError.invalidModelPath
            }

            return manifest
        } catch let error as ProjectError {
            throw error
        } catch {
            throw ProjectError.corruptManifest(error.localizedDescription)
        }
    }

    /// Synchronous save used by explicit user commands such as Cmd-S.
    func saveModel(_ model: MIR4DModelDocument, to projectURL: URL) throws {
        try ensurePackageDirectory(projectURL)
        let data = try makeEncoder().encode(model)
        try Self.writeModelData(data, to: projectURL)
    }

    /// Encodes a value snapshot and writes it away from the main actor.
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
        let url = projectURL.appendingPathComponent(Self.modelRelativePath)
        guard fileManager.fileExists(atPath: url.path) else {
            throw ProjectError.missingModel
        }
        do {
            let data = try Data(contentsOf: url)
            return try makeDecoder().decode(MIR4DModelDocument.self, from: data)
        } catch {
            throw ProjectError.corruptModel(error.localizedDescription)
        }
    }

    nonisolated private static func makeEncoder() -> JSONEncoder {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.sortedKeys, .withoutEscapingSlashes]
        encoder.dateEncodingStrategy = .iso8601
        return encoder
    }

    private func makeEncoder() -> JSONEncoder { Self.makeEncoder() }

    nonisolated private static func makeDecoder() -> JSONDecoder {
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        return decoder
    }

    private func makeDecoder() -> JSONDecoder { Self.makeDecoder() }

    nonisolated private static func writeModelData(_ data: Data, to projectURL: URL) throws {
        let fileManager = FileManager.default
        let url = projectURL.appendingPathComponent(MIR4DPaths.modelRelativePath)
        try fileManager.createDirectory(
            at: url.deletingLastPathComponent(),
            withIntermediateDirectories: true
        )
        try data.write(to: url, options: .atomic)
    }

    private func ensurePackageDirectory(_ projectURL: URL) throws {
        guard projectURL.pathExtension.lowercased() == Self.packageExtension else {
            throw ProjectError.invalidPackage
        }
        try fileManager.createDirectory(
            at: projectURL.appendingPathComponent("Scenes", isDirectory: true),
            withIntermediateDirectories: true
        )
    }

    private func isSafeRelativePath(_ path: String) -> Bool {
        guard !path.isEmpty, !path.hasPrefix("/"), !path.contains("..") else { return false }
        return !path.contains("\\")
    }

    private func sanitizedName(_ name: String) -> String {
        let forbidden = CharacterSet(charactersIn: "/:\\")
        let result = name
            .components(separatedBy: forbidden)
            .joined(separator: "-")
            .trimmingCharacters(in: .whitespacesAndNewlines)
        return result.isEmpty ? "Новый проект" : result
    }

    enum ProjectError: LocalizedError {
        case invalidName
        case alreadyExists
        case invalidPackage
        case unknownFormat(String)
        case unsupportedVersion(Int)
        case invalidModelPath
        case missingModel
        case corruptManifest(String)
        case corruptModel(String)

        var errorDescription: String? {
            switch self {
            case .invalidName: return "Некорректное имя проекта."
            case .alreadyExists: return "Проект с таким именем уже существует."
            case .invalidPackage: return "Это не пакет MIR 4D (.mir4d)."
            case .unknownFormat(let format): return "Неизвестный формат проекта: \(format)."
            case .unsupportedVersion(let version): return "Версия формата MIR 4D \(version) не поддерживается этой версией приложения."
            case .invalidModelPath: return "Манифест содержит недопустимый путь к модели."
            case .missingModel: return "В пакете MIR 4D отсутствует модель Scenes/model.mir4d.json."
            case .corruptManifest(let reason): return "Манифест проекта повреждён: \(reason)"
            case .corruptModel(let reason): return "Файл модели повреждён: \(reason)"
            }
        }
    }
}

/// Explicit value snapshot used to move model encoding to a utility task.
private struct MIR4DModelSnapshot: @unchecked Sendable {
    let model: MIR4DModelDocument
}
