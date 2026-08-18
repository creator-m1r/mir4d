import Foundation
import Combine

/// Runtime gatekeeper for optional MIR 4D subsystems.
/// A subsystem must be explicitly enabled by project consent before starting.
@MainActor
final class MIR4DOptionalModuleRuntime: ObservableObject {
    static let shared = MIR4DOptionalModuleRuntime()

    enum Module: String, CaseIterable, Identifiable {
        case camera
        case microphone
        case ai

        var id: String { rawValue }
    }

    @Published private(set) var enabledModules: Set<Module> = []
    @Published private(set) var runningModules: Set<Module> = []

    private var permissionObserver: NSObjectProtocol?

    init() {
        permissionObserver = NotificationCenter.default.addObserver(
            forName: .mir4DOptionalModulesConfigurationChanged,
            object: nil,
            queue: .main
        ) { [weak self] notification in
            Task { @MainActor in
                self?.apply(notification)
            }
        }
    }

    deinit {
        if let permissionObserver {
            NotificationCenter.default.removeObserver(permissionObserver)
        }
    }

    func apply(_ notification: Notification) {
        let camera = notification.userInfo?["cameraEnabled"] as? Bool ?? false
        let microphone = notification.userInfo?["microphoneEnabled"] as? Bool ?? false
        let ai = notification.userInfo?["aiEnabled"] as? Bool ?? false

        let requested: Set<Module> = [
            camera ? .camera : nil,
            microphone ? .microphone : nil,
            ai ? .ai : nil
        ].compactMap { $0 }

        enabledModules = requested

        for module in Module.allCases where !requested.contains(module) {
            stop(module)
        }
    }

    func isEnabled(_ module: Module) -> Bool {
        enabledModules.contains(module)
    }

    func start(_ module: Module) -> Bool {
        guard isEnabled(module) else { return false }
        runningModules.insert(module)
        return true
    }

    func stop(_ module: Module) {
        runningModules.remove(module)
    }

    func stopAll() {
        runningModules.removeAll()
    }
}
