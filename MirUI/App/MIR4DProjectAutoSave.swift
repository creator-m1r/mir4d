import Foundation
import Combine

@MainActor
final class MIR4DProjectAutoSave {
    private var timer: Timer?
    private weak var appState: CADAppState?
    private let interval: TimeInterval

    init(appState: CADAppState, interval: TimeInterval = 5.0) {
        self.appState = appState
        self.interval = interval
        start()
    }

    deinit {
        timer?.invalidate()
    }

    func start() {
        timer?.invalidate()
        timer = Timer.scheduledTimer(withTimeInterval: interval, repeats: true) { [weak self] _ in
            guard let self, let appState = self.appState else { return }
            guard appState.documentDirty else { return }
            appState.saveMIR4DProject()
        }
    }

    func stop() {
        timer?.invalidate()
        timer = nil
    }
}
