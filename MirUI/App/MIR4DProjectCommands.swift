import Foundation
import AppKit
import UniformTypeIdentifiers

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
        panel.allowedContentTypes = [.mir4dProject]
        panel.canCreateDirectories = true
        panel.isExtensionHidden = false

        guard panel.runModal() == .OK, let url = panel.url else { return }
        let name = url.deletingPathExtension().lastPathComponent
        appState.saveMIR4DProjectAs(parentURL: url.deletingLastPathComponent(), name: name)
    }

    /// Opens a project selected by the user from an Open panel.
    /// The Hub uses this same command path instead of touching the Store directly.
    func open(appState: CADAppState) {
        let panel = NSOpenPanel()
        panel.title = "Открыть проект MIR 4D"
        panel.message = "Выберите проект .mir4d"
        panel.allowedContentTypes = [.mir4dProject]
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false

        guard panel.runModal() == .OK, let url = panel.url else { return }
        open(appState: appState, url: url)
    }

    /// Opens a known project URL through the same Session path.
    /// LaunchCoordinator remains responsible for external macOS launch URLs.
    func open(appState: CADAppState, url: URL) {
        appState.openMIR4DProject(url: url)
    }

    func close(appState: CADAppState) {
        appState.closeMIR4DProject()
    }

    /// User-initiated Hub action. This is independent from the launch preference.
    func restoreLastProject(appState: CADAppState) -> Bool {
        MIR4DProjectSession.shared.continueLastProject(appState: appState)
    }

    /// Cold-start action. This is the only restore path that respects the
    /// user's "Открывать последний проект при запуске" preference.
    func restoreLastProjectOnLaunch(appState: CADAppState) -> Bool {
        MIR4DProjectSession.shared.restoreLastProjectOnLaunch(appState: appState)
    }
}

extension Notification.Name {
    static let mir4DExternalProjectURL = Notification.Name("MIR4D.ExternalProjectURL")
}
