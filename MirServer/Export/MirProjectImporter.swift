import Foundation

public struct MirProjectImporter: Sendable {
    public init() {}

    public func unpack(_ archive: Data, into destination: URL) throws -> URL {
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        let archive = try decoder.decode(MirProjectArchive.self, from: archive)

        let manager = FileManager.default
        try manager.createDirectory(at: destination, withIntermediateDirectories: true)
        for (relative, entry) in archive.entries {
            guard let data = Data(base64Encoded: entry.contentBase64) else { continue }
            let fileURL = destination.appendingPathComponent(relative)
            try manager.createDirectory(at: fileURL.deletingLastPathComponent(), withIntermediateDirectories: true)
            try data.write(to: fileURL, options: .atomic)
        }
        return destination
    }

    public func readManifest(_ archive: Data) throws -> MirProjectArchive {
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        return try decoder.decode(MirProjectArchive.self, from: archive)
    }
}
