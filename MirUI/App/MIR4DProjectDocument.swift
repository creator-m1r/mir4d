import Foundation

/// Canonical in-memory representation of a MIR 4D project.
/// The document is intentionally independent from SwiftUI views so the same
/// state can later be shared by MirEngine, the web client and collaboration services.
struct MIR4DProjectDocument: Codable, Equatable {
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

    static func makeNew(name: String) -> MIR4DProjectDocument {
        let now = Date()
        return MIR4DProjectDocument(
            name: name,
            createdAt: now,
            modifiedAt: now,
            workbench: "4D",
            subMode: "modelFeature",
            selectedTreeItem: "Проект",
            gridVisible: true,
            axesVisible: true,
            sectionMode: false,
            currentTime: 0
        )
    }
}
