import Foundation

/// Импорт опубликованного архива проекта MIR 4D из `Data` (см. MirProjectExporter).
///
/// Записывает содержимое архива обратно в каталог проекта на диске.
/// Модуль не зависит от UI и CAD-ядра; работает только с файловой системой.
public struct MirProjectImporter: Sendable {
    public init() {}

    /// Распаковать архив проекта в целевой каталог.
    /// - Parameter archive: данные архива, полученные с сервера.
    /// - Parameter destination: каталог назначения (будет создан при отсутствии).
    /// - Returns: URL распакованного каталога проекта.
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

    /// Декодировать только метаданные архива (без записи на диск).
    public func readManifest(_ archive: Data) throws -> MirProjectArchive {
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        return try decoder.decode(MirProjectArchive.self, from: archive)
    }
}
