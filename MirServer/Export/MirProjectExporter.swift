import Foundation

public struct MirProjectExporter: Sendable {
    public init() {}

    public func archiveProject(at projectURL: URL) throws -> Data {
        let manager = FileManager.default
        guard let enumerator = manager.enumerator(
            at: projectURL,
            includingPropertiesForKeys: [.isRegularFileKey, .fileSizeKey],
            options: [.skipsHiddenFiles]
        ) else {
            throw MirProjectExporterError.unreadableDirectory
        }

        var entries: [String: MirProjectArchiveEntry] = [:]
        let basePath = projectURL.standardizedFileURL.path
        for case let fileURL as URL in enumerator {
            let resource = try fileURL.resourceValues(forKeys: [.isRegularFileKey])
            guard resource.isRegularFile == true else { continue }
            let fullPath = fileURL.standardizedFileURL.path
            let relative = fullPath.replacingOccurrences(of: basePath + "/", with: "")
            guard let data = try? Data(contentsOf: fileURL) else { continue }
            entries[relative] = MirProjectArchiveEntry(
                size: data.count,
                contentBase64: data.base64EncodedString()
            )
        }

        let archive = MirProjectArchive(
            generator: "MIR4D MirServer",
            createdAt: Date(),
            root: projectURL.lastPathComponent,
            entries: entries
        )
        let encoder = JSONEncoder()
        encoder.dateEncodingStrategy = .iso8601
        encoder.outputFormatting = [.sortedKeys]
        return try encoder.encode(archive)
    }
}

public struct MirProjectArchiveEntry: Codable, Sendable {
    var size: Int
    var contentBase64: String
}

public struct MirProjectArchive: Codable, Sendable {
    var generator: String
    var createdAt: Date
    var root: String
    var entries: [String: MirProjectArchiveEntry]
}

public enum MirProjectExporterError: Error, Sendable, Equatable {
    case unreadableDirectory
}
