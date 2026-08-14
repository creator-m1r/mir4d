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
        panel.message = "Выберите имя и место для нового проекта."
        panel.nameFieldStringValue = appState.documentName
        panel.canCreateDirectories = true
        panel.isExtensionHidden = false
        panel.allowedContentTypes = [.folder]

        guard panel.runModal() == .OK, let url = panel.url else { return }
        appState.saveMIR4DProjectAs(parentURL: url.deletingLastPathComponent(), name: url.deletingPathExtension().lastPathComponent)
    }

    func close(appState: CADAppState) {
        MIR4DProjectSession.shared.close(appState: appState)
    }

    func restoreLastProject(appState: CADAppState) -> Bool {
        MIR4DProjectSession.shared.restoreLastProject(appState: appState)
    }
}
