import Foundation

@MainActor
final class MIR4DProjectSessionOptionalModulesBridge {
    static let shared = MIR4DProjectSessionOptionalModulesBridge()

    private let coordinator = MIR4DOptionalModuleCoordinator.shared
    private var observers: [NSObjectProtocol] = []

    private init() {
        let center = NotificationCenter.default

        observers.append(center.addObserver(
            forName: .mir4DProjectActivated,
            object: nil,
            queue: .main
        ) { [weak self] _ in
            Task { @MainActor in
                self?.coordinator.startConfiguredModules()
            }
        })

        observers.append(center.addObserver(
            forName: .mir4DProjectClosed,
            object: nil,
            queue: .main
        ) { [weak self] _ in
            Task { @MainActor in
                self?.coordinator.stopAllModules()
            }
        })
    }

    deinit {
        MainActor.assumeIsolated {
            observers.forEach(NotificationCenter.default.removeObserver)
        }
    }

    func install() {
        _ = Self.shared
    }
}
