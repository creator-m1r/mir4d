import Foundation

/// Debounced autosave for the active MIR 4D project.
///
/// A save is scheduled only after the model has been quiet for `debounceInterval`.
/// `minSaveInterval` prevents repeated writes when changes arrive in bursts.
@MainActor
final class MIR4DProjectAutoSave {
    private weak var appState: CADAppState?
    private var modelObserver: NSObjectProtocol?
    private var saveWorkItem: DispatchWorkItem?

    private let debounceInterval: TimeInterval
    private let minSaveInterval: TimeInterval
    private var lastSaveDate: Date?

    init(
        appState: CADAppState,
        debounceInterval: TimeInterval = 2.0,
        minSaveInterval: TimeInterval = 5.0
    ) {
        self.appState = appState
        self.debounceInterval = debounceInterval
        self.minSaveInterval = minSaveInterval

        modelObserver = NotificationCenter.default.addObserver(
            forName: .mir4DModelChanged,
            object: nil,
            queue: .main
        ) { [weak self] _ in
            Task { @MainActor in
                guard let self else { return }
                self.appState?.documentDirty = true
                self.scheduleSave()
            }
        }
    }

    deinit {
        saveWorkItem?.cancel()
        if let modelObserver {
            NotificationCenter.default.removeObserver(modelObserver)
        }
    }

    /// Schedule one save after the model has remained unchanged for the debounce interval.
    func scheduleSave() {
        guard let appState, appState.documentDirty else { return }

        saveWorkItem?.cancel()

        let work = DispatchWorkItem { [weak self] in
            guard let self else { return }
            self.performSaveIfNeeded()
        }
        saveWorkItem = work

        DispatchQueue.main.asyncAfter(
            deadline: .now() + debounceInterval,
            execute: work
        )
    }

    /// Cancel a pending automatic save.
    func stop() {
        saveWorkItem?.cancel()
        saveWorkItem = nil
    }

    /// Save immediately when the project is being closed or otherwise needs a flush.
    func flush() {
        saveWorkItem?.cancel()
        saveWorkItem = nil
        performSaveIfNeeded(force: true)
    }

    private func performSaveIfNeeded(force: Bool = false) {
        guard let appState, appState.documentDirty else { return }

        if !force,
           let lastSaveDate,
           Date().timeIntervalSince(lastSaveDate) < minSaveInterval {
            // Keep the dirty state and try again after the remaining cooldown.
            let remaining = max(
                0.05,
                minSaveInterval - Date().timeIntervalSince(lastSaveDate)
            )
            saveWorkItem?.cancel()
            let work = DispatchWorkItem { [weak self] in
                self?.performSaveIfNeeded()
            }
            saveWorkItem = work
            DispatchQueue.main.asyncAfter(deadline: .now() + remaining, execute: work)
            return
        }

        appState.saveMIR4DProject()
        lastSaveDate = Date()
        saveWorkItem = nil
    }
}
