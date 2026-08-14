import Foundation
import Combine

@MainActor
final class MIR4DProjectAutoSave {
    private var timer: Timer?
    private var modelObserver: NSObjectProtocol?
    private weak var appState: CADAppState?
    private let interval: TimeInterval

    init(appState: CADAppState, interval: TimeInterval = 5.0) {
        self.appState = appState
        self.interval = interval
        modelObserver = NotificationCenter.default.addObserver(forName: .mir4DModelChanged, object: nil, queue: .main) { [weak self] _ in
            self?.appState?.documentDirty = true
        }
        start()
    }

    deinit {
        timer?.invalidate()
        if let modelObserver { NotificationCenter.default.removeObserver(modelObserver) }
    }

    func start() {
        timer?.invalidate()
        timer = Timer.scheduledTimer(withTimeInterval: interval, repeats: true) { [weak self] _ in
            guard let self, let appState = self.appState, appState.documentDirty else { return }
            appState.saveMIR4DProject()
        }
    }

    func stop() {
        timer?.invalidate()
        timer = nil
    }
}
