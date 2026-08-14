import Foundation

@MainActor
extension MIR4DProjectSession {
    /// Opens the remembered project without consulting the auto-open preference.
    /// The Hub uses this semantic; cold-start policy stays in ProjectCommands.
    func continueLastProject(appState: CADAppState) -> Bool {
        guard let url = lastProjectURL() else { return false }
        guard MIR4DProjectStore.shared.isValidPackage(at: url) else {
            appState.showNotification(
                "Последний проект недоступен. Выберите проект из списка недавних.",
                type: .warning
            )
            return false
        }
        openProject(appState: appState, url: url)
        return true
    }

    /// Opens the remembered project only when launch auto-open is enabled.
    /// This is deliberately separate from the user-initiated Hub Continue action.
    func restoreLastProjectOnLaunch(appState: CADAppState) -> Bool {
        guard isAutoOpenLastProjectEnabled else { return false }
        return continueLastProject(appState: appState)
    }
}
