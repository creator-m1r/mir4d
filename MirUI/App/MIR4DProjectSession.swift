import Foundation
import AppKit

extension Notification.Name {
    static let mir4DProjectActivated = Notification.Name("MIR4D.ProjectActivated")
    static let mir4DRequestNewProject = Notification.Name("MIR4D.RequestNewProject")
    static let mir4DProjectSaved = Notification.Name("MIR4D.ProjectSaved")
}

@MainActor
final class MIR4DProjectSession {
    static let shared = MIR4DProjectSession()

    private(set) var projectURL: URL?
    private(set) var projectName: String = "Новый проект"
    private var autoSave: MIR4DProjectAutoSave?

    private init() {}

    func createProject(appState: CADAppState, name: String, parentURL: URL) {
        do {
            let url = try MIR4DProjectStore.shared.createProject(name: name, in: parentURL)
            projectURL = url
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

            try save(appState: appState)
            startAutoSave(for: appState)

            appState.showNotification(
                "Проект создан: \(projectName)",
                type: .success
            )

            NotificationCenter.default.post(
                name: .mir4DProjectActivated,
                object: url
            )
        } catch {
            appState.showNotification(
                "Не удалось создать проект: \(error.localizedDescription)",
                type: .error
            )
        }
    }

    func openProject(appState: CADAppState, url: URL) {
        do {
            let manifest = try MIR4DProjectStore.shared.load(from: url)
            projectURL = url
            projectName = manifest.name

            appState.documentName = manifest.name
            appState.documentDirty = false
            appState.selectedTreeItem = manifest.selectedTreeItem
            appState.gridVisible = manifest.gridVisible
            appState.axesVisible = manifest.axesVisible
            appState.sectionMode = manifest.sectionMode
            appState.time.seek(manifest.currentTime)

            if let workbench = CADWorkbench(rawValue: manifest.workbench) {
                appState.workbench = workbench
            }

            if let subMode = CADSubMode(rawValue: manifest.subMode) {
                appState.subMode = subMode
            }

            startAutoSave(for: appState)

            appState.showNotification(
                "Проект открыт: \(manifest.name)",
                type: .success
            )

            NotificationCenter.default.post(
                name: .mir4DProjectActivated,
                object: url
            )
        } catch {
            appState.showNotification(
                "Не удалось открыть проект: \(error.localizedDescription)",
                type: .error
            )
        }
    }

    func save(appState: CADAppState) throws {
        guard let projectURL else {
            return
        }

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

        try MIR4DProjectStore.shared.save(manifest: manifest, to: projectURL)
        appState.documentDirty = false
        NotificationCenter.default.post(name: .mir4DProjectSaved, object: projectURL)
    }

    private func startAutoSave(for appState: CADAppState) {
        autoSave?.stop()
        autoSave = MIR4DProjectAutoSave(appState: appState)
    }

    private func existingCreationDate(in projectURL: URL) -> Date {
        guard let manifest = try? MIR4DProjectStore.shared.load(from: projectURL) else {
            return Date()
        }
        return manifest.createdAt
    }
}

extension CADAppState {
    func createMIR4DProject(name: String, parentURL: URL) {
        MIR4DProjectSession.shared.createProject(
            appState: self,
            name: name,
            parentURL: parentURL
        )
    }

    func openMIR4DProject(url: URL) {
        MIR4DProjectSession.shared.openProject(
            appState: self,
            url: url
        )
    }

    func saveMIR4DProject() {
        do {
            try MIR4DProjectSession.shared.save(appState: self)
        } catch {
            showNotification(
                "Не удалось сохранить проект: \(error.localizedDescription)",
                type: .error
            )
        }
    }
}
