import Foundation
import AppKit

@MainActor
final class MIR4DProjectCommands {
    static let shared = MIR4DProjectCommands()
    private init() {}

    func save(appState: CADAppState) {
        appState.saveMIR4DProject()
    }

    func saveAs(appState: CADAppState) {
        let panel = NSSavePanel()
        panel.title = "Сохранить проект MIR 4D как"
        panel.message = "Выберите имя проекта и место его хранения."
        panel.nameFieldStringValue = "\(appState.documentName).mir4d"
        panel.canCreateDirectories = true
        panel.isExtensionHidden = false

        guard panel.runModal() == .OK, let url = panel.url else { return }
        let name = url.deletingPathExtension().lastPathComponent
        appState.saveMIR4DProjectAs(parentURL: url.deletingLastPathComponent(), name: name)
    }

    func close(appState: CADAppState) {
        appState.closeMIR4DProject()
    }

    func restoreLastProject(appState: CADAppState) -> Bool {
        MIR4DProjectSession.shared.restoreLastProject(appState: appState)
    }
}
