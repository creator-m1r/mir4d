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

    func open(appState: CADAppState, url: URL) {
        appState.openMIR4DProject(url: url)
    }

    func close(appState: CADAppState) {
        appState.closeMIR4DProject()
    }

    func exportStep(appState: CADAppState) {
        let panel = NSSavePanel()
        panel.title = "Сохранить модель как STEP"
        panel.message = "Экспорт созданных моделей в формат STEP."
        panel.nameFieldStringValue = "\(appState.documentName).step"
        panel.allowedContentTypes = [UTType(filenameExtension: "step")].compactMap { $0 }
        panel.canCreateDirectories = true
        panel.isExtensionHidden = false

        guard panel.runModal() == .OK, let url = panel.url else { return }
        NotificationCenter.default.post(
            name: .mir4DExportStep,
            object: [
                "path": url.path,
                "selectionOnly": false
            ]
        )
    }

    func exportStepBRep(appState: CADAppState) {
        let panel = NSSavePanel()
        panel.title = "Сохранить точный B-Rep как STEP"
        panel.message = "Экспорт точной B-Rep геометрии в формат STEP."
        panel.nameFieldStringValue = "\(appState.documentName).step"
        panel.allowedContentTypes = [UTType(filenameExtension: "step")].compactMap { $0 }
        panel.canCreateDirectories = true
        panel.isExtensionHidden = false

        guard panel.runModal() == .OK, let url = panel.url else { return }
        NotificationCenter.default.post(
            name: .mir4DExportStepBRep,
            object: [
                "path": url.path,
                "selectionOnly": false
            ]
        )
    }

    func restoreLastProject(appState: CADAppState) -> Bool {
        MIR4DProjectSession.shared.continueLastProject(appState: appState)
    }

    func restoreLastProjectOnLaunch(appState: CADAppState) -> Bool {
        MIR4DProjectSession.shared.restoreLastProjectOnLaunch(appState: appState)
    }
}

extension Notification.Name {
    static let mir4DExternalProjectURL = Notification.Name("MIR4D.ExternalProjectURL")
}
