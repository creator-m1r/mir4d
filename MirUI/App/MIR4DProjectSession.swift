import Foundation
import AppKit

extension Notification.Name {
    static let mir4DProjectActivated = Notification.Name("MIR4D.ProjectActivated")
    static let mir4DRequestNewProject = Notification.Name("MIR4D.RequestNewProject")
    static let mir4DProjectSaved = Notification.Name("MIR4D.ProjectSaved")
    static let mir4DProjectClosed = Notification.Name("MIR4D.ProjectClosed")
    static let mir4DProjectRestoreRequested = Notification.Name("MIR4D.ProjectRestoreRequested")
}

enum MIR4DSaveScope {
    case manifestOnly
    case modelOnly
    case full
}

struct MIR4DRecentProject: Codable, Identifiable, Equatable {
    let id: UUID
    let name: String
    let path: String
    let lastOpened: Date

    var url: URL { URL(fileURLWithPath: path, isDirectory: true) }
}

@MainActor
final class MIR4DProjectSession {
    static let shared = MIR4DProjectSession()
    private(set) var projectURL: URL?
    private(set) var projectName: String = "Новый проект"
    private(set) var projectUUID: UUID?
    private var autoSave: MIR4DProjectAutoSave?
    private let lastProjectDefaultsKey = "MIR4D.lastProjectURL"
    private let autoOpenLastKey = "MIR4D.autoOpenLastProject"
    private let recentProjectsKey = "MIR4D.recentProjects"
    private let maxRecentProjects = 10
    private let modelRuntime = MIR4DModelRuntime.shared
    private var lastSavedModelRevision: UInt64 = 0

    private init() {}

    var isAutoOpenLastProjectEnabled: Bool {
        get { UserDefaults.standard.object(forKey: autoOpenLastKey) as? Bool ?? true }
        set { UserDefaults.standard.set(newValue, forKey: autoOpenLastKey) }
    }

    var recentProjects: [MIR4DRecentProject] { loadRecentProjects() }

    func createProject(appState: CADAppState, name: String, parentURL: URL) {
        do {
            let url = try MIR4DProjectStore.shared.createProject(name: name, in: parentURL)
            projectUUID = UUID()
            activate(url: url, name: name, appState: appState)
            modelRuntime.reset(projectName: appState.documentName)
            try save(appState: appState)
            startAutoSave(for: appState)
            notifyActivation(url: url, appState: appState, message: "Проект создан: \(projectName)")
        } catch { appState.showNotification("Не удалось создать проект: \(error.localizedDescription)", type: .error) }
    }

    func openProject(appState: CADAppState, url: URL) {
        do {
            let manifest = try MIR4DProjectStore.shared.load(from: url)
            projectURL = url.standardizedFileURL
            projectName = manifest.name
            // Soft migration: old v1 manifests may not have an identity yet.
            // Generate it once and persist it on the next save.
            projectUUID = manifest.uuid ?? UUID()
            appState.documentName = manifest.name
            appState.documentDirty = manifest.uuid == nil
            appState.selectedTreeItem = manifest.selectedTreeItem
            appState.gridVisible = manifest.gridVisible
            appState.axesVisible = manifest.axesVisible
            appState.sectionMode = manifest.sectionMode
            appState.time.seek(manifest.currentTime)
            if let workbench = CADWorkbench(rawValue: manifest.workbench) { appState.workbench = workbench }
            if let subMode = CADSubMode(rawValue: manifest.subMode) { appState.subMode = subMode }
            if let model = try? MIR4DProjectStore.shared.loadModel(from: url) {
                modelRuntime.load(model)
                lastSavedModelRevision = modelRuntime.revision
                appState.showNotification("Модель загружена: \(model.geometry.count) объектов", type: .success)
            } else {
                modelRuntime.reset(projectName: manifest.name)
                try MIR4DProjectStore.shared.saveModel(modelRuntime.document, to: url)
                lastSavedModelRevision = modelRuntime.revision
            }
            addToRecents(url: url, name: manifest.name)
            startAutoSave(for: appState)
            notifyActivation(url: url, appState: appState, message: "Проект открыт: \(manifest.name)")
        } catch {
            appState.showNotification("Не удалось открыть проект: \(error.localizedDescription)", type: .error)
        }
    }

    func save(appState: CADAppState) throws {
        try saveSync(appState: appState, scope: .full)
    }

    func saveSync(appState: CADAppState, scope: MIR4DSaveScope = .full) throws {
        guard let projectURL else { throw ProjectError.noActiveProject }

        switch scope {
        case .manifestOnly:
            try MIR4DProjectStore.shared.save(manifest: makeManifest(from: appState, in: projectURL), to: projectURL)
        case .modelOnly:
            guard modelRuntime.revision != lastSavedModelRevision else { return }
            try MIR4DProjectStore.shared.saveModel(modelRuntime.document, to: projectURL)
            lastSavedModelRevision = modelRuntime.revision
        case .full:
            try MIR4DProjectStore.shared.save(manifest: makeManifest(from: appState, in: projectURL), to: projectURL)
            if modelRuntime.revision != lastSavedModelRevision {
                try MIR4DProjectStore.shared.saveModel(modelRuntime.document, to: projectURL)
                lastSavedModelRevision = modelRuntime.revision
            }
        }

        appState.documentDirty = false
        addToRecents(url: projectURL, name: appState.documentName)
        NotificationCenter.default.post(name: .mir4DProjectSaved, object: projectURL)
    }

    func saveAsync(appState: CADAppState, scope: MIR4DSaveScope = .full) async throws {
        guard let projectURL else { throw ProjectError.noActiveProject }

        let manifest: MIR4DProjectManifest?
        switch scope {
        case .manifestOnly, .full: manifest = makeManifest(from: appState, in: projectURL)
        case .modelOnly: manifest = nil
        }

        let modelSnapshot: MIR4DModelDocument?
        let revision = modelRuntime.revision
        switch scope {
        case .modelOnly, .full:
            guard revision != lastSavedModelRevision else {
                if scope == .modelOnly { return }
                modelSnapshot = nil
                break
            }
            modelSnapshot = modelRuntime.document
        case .manifestOnly:
            modelSnapshot = nil
        }

        if let manifest {
            try MIR4DProjectStore.shared.save(manifest: manifest, to: projectURL)
        }
        if let modelSnapshot {
            try await MIR4DProjectStore.shared.saveModelAsync(modelSnapshot, to: projectURL)
            lastSavedModelRevision = revision
        }

        appState.documentDirty = false
        addToRecents(url: projectURL, name: appState.documentName)
        NotificationCenter.default.post(name: .mir4DProjectSaved, object: projectURL)
    }

    func saveAs(appState: CADAppState, name: String, parentURL: URL) {
        do {
            let url = try MIR4DProjectStore.shared.createProject(name: name, in: parentURL)
            projectUUID = UUID()
            projectURL = url
            projectName = name.trimmingCharacters(in: .whitespacesAndNewlines)
            appState.documentName = projectName
            lastSavedModelRevision = 0
            try save(appState: appState)
            startAutoSave(for: appState)
            appState.showNotification("Проект сохранён как: \(projectName)", type: .success)
            NotificationCenter.default.post(name: .mir4DProjectActivated, object: url)
        } catch { appState.showNotification("Не удалось сохранить проект как: \(error.localizedDescription)", type: .error) }
    }

    func close(appState: CADAppState) {
        autoSave?.flush()
        autoSave?.stop()
        autoSave = nil
        projectURL = nil
        projectUUID = nil
        projectName = "Новый проект"
        lastSavedModelRevision = 0
        modelRuntime.reset(projectName: "Новый проект")
        appState.documentName = "Новый проект"
        appState.documentDirty = false
        NotificationCenter.default.post(name: .mir4DProjectClosed, object: nil)
    }

    func restoreLastProject(appState: CADAppState) -> Bool {
        guard isAutoOpenLastProjectEnabled,
              let path = UserDefaults.standard.string(forKey: lastProjectDefaultsKey)
        else { return false }

        let url = URL(fileURLWithPath: path, isDirectory: true).standardizedFileURL
        guard MIR4DProjectStore.shared.isValidPackage(at: url) else {
            clearLastProject()
            appState.showNotification("Последний проект недоступен. Открыт стартовый экран.", type: .warning)
            return false
        }

        do {
            _ = try MIR4DProjectStore.shared.load(from: url)
        } catch {
            clearLastProject()
            appState.showNotification("Последний проект недоступен. Открыт стартовый экран.", type: .warning)
            return false
        }

        openProject(appState: appState, url: url)
        return true
    }

    func removeRecentProject(_ project: MIR4DRecentProject) {
        var list = loadRecentProjects()
        list.removeAll { $0.path == project.path }
        saveRecentProjects(list)
        if UserDefaults.standard.string(forKey: lastProjectDefaultsKey) == project.path { clearLastProject() }
    }

    func clearLastProject() { UserDefaults.standard.removeObject(forKey: lastProjectDefaultsKey) }

    func scheduleAutoSave() { autoSave?.scheduleSave() }

    func requestClose(appState: CADAppState, confirm: @escaping (Bool) -> Void) {
        guard appState.documentDirty else { close(appState: appState); confirm(true); return }
        let alert = NSAlert()
        alert.messageText = "Сохранить изменения в проекте?"
        alert.informativeText = "В проекте «\(appState.documentName)» есть несохранённые изменения."
        alert.addButton(withTitle: "Сохранить")
        alert.addButton(withTitle: "Не сохранять")
        alert.addButton(withTitle: "Отмена")
        switch alert.runModal() {
        case .alertFirstButtonReturn:
            autoSave?.flush()
            close(appState: appState)
            confirm(true)
        case .alertSecondButtonReturn:
            close(appState: appState)
            confirm(true)
        default: confirm(false)
        }
    }

    private func activate(url: URL, name: String, appState: CADAppState) {
        projectURL = url.standardizedFileURL
        projectName = name.trimmingCharacters(in: .whitespacesAndNewlines)
        appState.documentName = projectName
        appState.documentDirty = false
        appState.selectedTreeItem = "Проект"
        appState.gridVisible = true
        appState.axesVisible = true
        appState.sectionMode = false
        appState.workbench = .fourD
        appState.subMode = .modelFeature
        appState.time.reset()
        addToRecents(url: url, name: projectName)
    }

    private func notifyActivation(url: URL, appState: CADAppState, message: String) {
        appState.showNotification(message, type: .success)
        NotificationCenter.default.post(name: .mir4DProjectActivated, object: url)
    }

    private func startAutoSave(for appState: CADAppState) {
        autoSave?.stop()
        autoSave = MIR4DProjectAutoSave(appState: appState)
    }

    private func makeManifest(from appState: CADAppState, in projectURL: URL) -> MIR4DProjectManifest {
        MIR4DProjectManifest(
            name: appState.documentName,
            createdAt: existingCreationDate(in: projectURL),
            modifiedAt: Date(),
            workbench: appState.workbench.rawValue,
            subMode: appState.subMode.rawValue,
            selectedTreeItem: appState.selectedTreeItem,
            gridVisible: appState.gridVisible,
            axesVisible: appState.axesVisible,
            sectionMode: appState.sectionMode,
            currentTime: appState.currentTime,
            uuid: projectUUID
        )
    }

    private func existingCreationDate(in projectURL: URL) -> Date {
        guard let manifest = try? MIR4DProjectStore.shared.load(from: projectURL) else { return Date() }
        return manifest.createdAt
    }

    private func loadRecentProjects() -> [MIR4DRecentProject] {
        guard let data = UserDefaults.standard.data(forKey: recentProjectsKey),
              let list = try? JSONDecoder().decode([MIR4DRecentProject].self, from: data)
        else { return [] }
        return list.filter {
            FileManager.default.fileExists(atPath: $0.url.path) &&
            FileManager.default.fileExists(atPath: $0.url.appendingPathComponent("project.mir4d.json").path)
        }
    }

    private func addToRecents(url: URL, name: String) {
        var list = loadRecentProjects()
        list.removeAll { $0.path == url.path }
        list.insert(MIR4DRecentProject(id: projectUUID ?? UUID(), name: name, path: url.path, lastOpened: Date()), at: 0)
        if list.count > maxRecentProjects { list = Array(list.prefix(maxRecentProjects)) }
        saveRecentProjects(list)
        UserDefaults.standard.set(url.path, forKey: lastProjectDefaultsKey)
    }

    private func saveRecentProjects(_ list: [MIR4DRecentProject]) {
        if let data = try? JSONEncoder().encode(list) { UserDefaults.standard.set(data, forKey: recentProjectsKey) }
    }

    enum ProjectError: LocalizedError {
        case noActiveProject
        var errorDescription: String? { "Нет активного проекта MIR 4D." }
    }
}

extension CADAppState {
    func createMIR4DProject(name: String, parentURL: URL) { MIR4DProjectSession.shared.createProject(appState: self, name: name, parentURL: parentURL) }
    func openMIR4DProject(url: URL) { MIR4DProjectSession.shared.openProject(appState: self, url: url) }
    func saveMIR4DProject() { do { try MIR4DProjectSession.shared.save(appState: self) } catch { showNotification("Не удалось сохранить проект: \(error.localizedDescription)", type: .error) } }
    func saveMIR4DProjectAs(parentURL: URL, name: String) { MIR4DProjectSession.shared.saveAs(appState: self, name: name, parentURL: parentURL) }
    func closeMIR4DProject(confirm: @escaping (Bool) -> Void = { _ in }) { MIR4DProjectSession.shared.requestClose(appState: self, confirm: confirm) }
}
