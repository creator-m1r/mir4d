import Foundation
import AppKit

extension Notification.Name {
    static let mir4DProjectActivated = Notification.Name("MIR4D.ProjectActivated")
    static let mir4DRequestNewProject = Notification.Name("MIR4D.RequestNewProject")
    static let mir4DProjectSaved = Notification.Name("MIR4D.ProjectSaved")
    static let mir4DProjectClosed = Notification.Name("MIR4D.ProjectClosed")
    static let mir4DProjectRestoreRequested = Notification.Name("MIR4D.ProjectRestoreRequested")
}

@MainActor
final class MIR4DProjectSession {
    static let shared = MIR4DProjectSession()

    private(set) var projectURL: URL?
    private(set) var projectName: String = "Новый проект"
    private var autoSave: MIR4DProjectAutoSave?
    private let lastProjectDefaultsKey = "MIR4D.lastProjectURL"

    private init() {}

    func createProject(appState: CADAppState, name: String, parentURL: URL) {
        do {
            let url = try MIR4DProjectStore.shared.createProject(name: name, in: parentURL)
            activate(url: url, name: name, appState: appState)
            try save(appState: appState)
            startAutoSave(for: appState)
            notifyActivation(url: url, appState: appState, message: "Проект создан: \(projectName)")
        } catch {
            appState.showNotification("Не удалось создать проект: \(error.localizedDescription)", type: .error)
        }
    }

    func openProject(appState: CADAppState, url: URL) {
        do {
            let manifest = try MIR4DProjectStore.shared.load(from: url)
            projectURL = url
            projectName = manifest.name
            UserDefaults.standard.set(url.path, forKey: lastProjectDefaultsKey)

            appState.documentName = manifest.name
            appState.documentDirty = false
            appState.selectedTreeItem = manifest.selectedTreeItem
            appState.gridVisible = manifest.gridVisible
            appState.axesVisible = manifest.axesVisible
            appState.sectionMode = manifest.sectionMode
            appState.time.seek(manifest.currentTime)

            if let workbench = CADWorkbench(rawValue: manifest.workbench) { appState.workbench = workbench }
            if let subMode = CADSubMode(rawValue: manifest.subMode) { appState.subMode = subMode }

            if let model = try? MIR4DProjectStore.shared.loadModel(from: url) {
                appState.showNotification("Модель загружена: \(model.root.title)", type: .success)
            } else {
                // Backward compatibility: older .mir4d projects only contain the manifest.
                let model = MIR4DModelDocument.from(tree: appState.treeData, projectName: manifest.name)
                try MIR4DProjectStore.shared.saveModel(model, to: url)
            }

            startAutoSave(for: appState)
            notifyActivation(url: url, appState: appState, message: "Проект открыт: \(manifest.name)")
        } catch {
            appState.showNotification("Не удалось открыть проект: \(error.localizedDescription)", type: .error)
        }
    }

    func save(appState: CADAppState) throws {
        guard let projectURL else { throw ProjectError.noActiveProject }

        let manifest = MIR4DProjectManifest(
            name: appState.documentName,
            createdAt: existingCreationDate(in: projectURL),
            modifiedAt: Date(),
            workbench: appState.workbench.rawValue,
            subMode: appState.subMode.rawValue,
            selectedTreeItem: appState.selectedTreeItem,
            gridVisible: appState.gridVisible,
            axesVisible: appState.axesVisible,
            sectionMode: appState.sectionMode,
            currentTime: appState.currentTime
        )

        let model = MIR4DModelDocument.from(tree: appState.treeData, projectName: appState.documentName)
        try MIR4DProjectStore.shared.save(manifest: manifest, to: projectURL)
        try MIR4DProjectStore.shared.saveModel(model, to: projectURL)

        appState.documentDirty = false
        UserDefaults.standard.set(projectURL.path, forKey: lastProjectDefaultsKey)
        NotificationCenter.default.post(name: .mir4DProjectSaved, object: projectURL)
    }

    func saveAs(appState: CADAppState, name: String, parentURL: URL) {
        do {
            let url = try MIR4DProjectStore.shared.createProject(name: name, in: parentURL)
            projectURL = url
            projectName = name.trimmingCharacters(in: .whitespacesAndNewlines)
            appState.documentName = projectName
            try save(appState: appState)
            startAutoSave(for: appState)
            appState.showNotification("Проект сохранён как: \(projectName)", type: .success)
            NotificationCenter.default.post(name: .mir4DProjectActivated, object: url)
        } catch {
            appState.showNotification("Не удалось сохранить проект как: \(error.localizedDescription)", type: .error)
        }
    }

    func close(appState: CADAppState) {
        autoSave?.stop()
        autoSave = nil
        projectURL = nil
        projectName = "Новый проект"
        appState.documentName = "Новый проект"
        appState.documentDirty = false
        NotificationCenter.default.post(name: .mir4DProjectClosed, object: nil)
    }

    func restoreLastProject(appState: CADAppState) -> Bool {
        guard let path = UserDefaults.standard.string(forKey: lastProjectDefaultsKey) else { return false }
        let url = URL(fileURLWithPath: path, isDirectory: true)
        guard FileManager.default.fileExists(atPath: url.appendingPathComponent("project.mir4d.json").path) else { return false }
        openProject(appState: appState, url: url)
        return true
    }

    func requestClose(appState: CADAppState, confirm: @escaping (Bool) -> Void) {
        guard appState.documentDirty else {
            close(appState: appState)
            confirm(true)
            return
        }

        let alert = NSAlert()
        alert.messageText = "Сохранить изменения в проекте?"
        alert.informativeText = "В проекте «\(appState.documentName)» есть несохранённые изменения."
        alert.addButton(withTitle: "Сохранить")
        alert.addButton(withTitle: "Не сохранять")
        alert.addButton(withTitle: "Отмена")

        switch alert.runModal() {
        case .alertFirstButtonReturn:
            appState.saveMIR4DProject()
            close(appState: appState)
            confirm(true)
        case .alertSecondButtonReturn:
            close(appState: appState)
            confirm(true)
        default:
            confirm(false)
        }
    }

    private func activate(url: URL, name: String, appState: CADAppState) {
        projectURL = url
        projectName = name.trimmingCharacters(in: .whitespacesAndNewlines)
        UserDefaults.standard.set(url.path, forKey: lastProjectDefaultsKey)
        appState.documentName = projectName
        appState.documentDirty = false
        appState.selectedTreeItem = "Проект"
        appState.gridVisible = true
        appState.axesVisible = true
        appState.sectionMode = false
        appState.workbench = .fourD
        appState.subMode = .modelFeature
        appState.time.reset()
    }

    private func notifyActivation(url: URL, appState: CADAppState, message: String) {
        appState.showNotification(message, type: .success)
        NotificationCenter.default.post(name: .mir4DProjectActivated, object: url)
    }

    private func startAutoSave(for appState: CADAppState) {
        autoSave?.stop()
        autoSave = MIR4DProjectAutoSave(appState: appState)
    }

    private func existingCreationDate(in projectURL: URL) -> Date {
        guard let manifest = try? MIR4DProjectStore.shared.load(from: projectURL) else { return Date() }
        return manifest.createdAt
    }

    enum ProjectError: LocalizedError {
        case noActiveProject
        var errorDescription: String? { "Нет активного проекта MIR 4D." }
    }
}

extension CADAppState {
    func createMIR4DProject(name: String, parentURL: URL) { MIR4DProjectSession.shared.createProject(appState: self, name: name, parentURL: parentURL) }
    func openMIR4DProject(url: URL) { MIR4DProjectSession.shared.openProject(appState: self, url: url) }
    func saveMIR4DProject() {
        do { try MIR4DProjectSession.shared.save(appState: self) }
        catch { showNotification("Не удалось сохранить проект: \(error.localizedDescription)", type: .error) }
    }
    func saveMIR4DProjectAs(parentURL: URL, name: String) { MIR4DProjectSession.shared.saveAs(appState: self, name: name, parentURL: parentURL) }
    func closeMIR4DProject(confirm: @escaping (Bool) -> Void = { _ in }) { MIR4DProjectSession.shared.requestClose(appState: self, confirm: confirm) }
}
