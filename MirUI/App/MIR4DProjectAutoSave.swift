import Foundation

@MainActor
final class MIR4DProjectAutoSave {
    private weak var appState: CADAppState?

    nonisolated(unsafe) private var modelObserver: NSObjectProtocol?
    nonisolated(unsafe) private var saveWorkItem: DispatchWorkItem?

    private let debounceInterval: TimeInterval
    private let minSaveInterval: TimeInterval
    private var lastSaveDate: Date?
    private var isSaving = false
    private var needsResave = false

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

    func scheduleSave() {
        guard let appState, appState.documentDirty else { return }

        if isSaving {

            needsResave = true
            return
        }

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

    func stop() {
        saveWorkItem?.cancel()
        saveWorkItem = nil
        needsResave = false
    }

    func flush() {
        saveWorkItem?.cancel()
        saveWorkItem = nil
        performSaveIfNeeded(force: true)
    }

    private func performSaveIfNeeded(force: Bool = false) {
        guard let appState, appState.documentDirty else { return }

        if isSaving {
            needsResave = true
            return
        }

        if !force,
           let lastSaveDate,
           Date().timeIntervalSince(lastSaveDate) < minSaveInterval {
            let remaining = max(0.05, minSaveInterval - Date().timeIntervalSince(lastSaveDate))
            saveWorkItem?.cancel()
            let work = DispatchWorkItem { [weak self] in
                self?.performSaveIfNeeded()
            }
            saveWorkItem = work
            DispatchQueue.main.asyncAfter(deadline: .now() + remaining, execute: work)
            return
        }

        isSaving = true
        needsResave = false
        saveWorkItem = nil

        Task { @MainActor [weak self, weak appState] in
            guard let self, let appState else { return }
            do {

                try await MIR4DProjectSession.shared.saveAsync(appState: appState, scope: .modelOnly)
                self.lastSaveDate = Date()
            } catch {
                appState.showNotification(
                    "Автосохранение не удалось: \(error.localizedDescription)",
                    type: .warning
                )
            }

            self.isSaving = false

            if self.needsResave || appState.documentDirty {
                self.needsResave = false
                self.scheduleSave()
            }
        }
    }
}
