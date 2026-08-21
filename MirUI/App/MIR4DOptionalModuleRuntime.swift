import Foundation
import Combine

/// Runtime gatekeeper for optional MIR 4D subsystems.
/// A subsystem must be explicitly enabled by project consent before starting.
@MainActor
final class MIR4DOptionalModuleRuntime: ObservableObject {
    static let shared = MIR4DOptionalModuleRuntime()

    @Published private(set) var enabledModules: Set<MIR4DOptionalModule> = []
    @Published private(set) var runningModules: Set<MIR4DOptionalModule> = []

    private var permissionObserver: NSObjectProtocol?

    init() {
        permissionObserver = NotificationCenter.default.addObserver(
            forName: .mir4DOptionalModulesConfigurationChanged,
            object: nil,
            queue: .main
        ) { [weak self] notification in
            let camera = notification.userInfo?["cameraEnabled"] as? Bool ?? false
            let microphone = notification.userInfo?["microphoneEnabled"] as? Bool ?? false
            let ai = notification.userInfo?["aiEnabled"] as? Bool ?? false
            MainActor.assumeIsolated {
                self?.apply(camera: camera, microphone: microphone, ai: ai)
            }
        }
    }

    deinit {
        MainActor.assumeIsolated {
            if let permissionObserver {
                NotificationCenter.default.removeObserver(permissionObserver)
            }
        }
    }

    /// Applies the enabled-module set posted by the permissions UI.
    func apply(_ modules: Set<MIR4DOptionalModule>) {
        enabledModules = modules
        for module in MIR4DOptionalModule.allCases where !modules.contains(module) {
            stop(module)
        }
    }

    /// Applies a configuration-change notification payload (cameraEnabled / microphoneEnabled / aiEnabled).
    func apply(camera: Bool, microphone: Bool, ai: Bool) {
        let requested: [MIR4DOptionalModule?] = [
            camera ? .camera : nil,
            microphone ? .microphone : nil,
            ai ? .ai : nil
        ]
        apply(Set(requested.compactMap { $0 }))
    }

    func isEnabled(_ module: MIR4DOptionalModule) -> Bool {
        enabledModules.contains(module)
    }

    /// Starts the module if enabled. Returns whether it is now running.
    func start(_ module: MIR4DOptionalModule) -> Bool {
        guard isEnabled(module) else { return false }
        runningModules.insert(module)
        return true
    }

    func stop(_ module: MIR4DOptionalModule) {
        runningModules.remove(module)
    }

    func stopAll() {
        runningModules.removeAll()
    }
}
