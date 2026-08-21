import Foundation
import Combine

/// Coordinates lifecycle of optional subsystems without coupling them to UI.
/// Camera permission controls both camera and hand-tracking agents.
@MainActor
final class MIR4DOptionalModuleCoordinator: ObservableObject {
    static let shared = MIR4DOptionalModuleCoordinator()

    private let runtime = MIR4DOptionalModuleRuntime.shared

    @Published private(set) var cameraRunning = false
    @Published private(set) var microphoneRunning = false
    @Published private(set) var aiRunning = false

    func startConfiguredModules() {
        cameraRunning = runtime.start(.camera)
        microphoneRunning = runtime.start(.microphone)
        aiRunning = runtime.start(.ai)
    }

    func stopAllModules() {
        runtime.stopAll()
        cameraRunning = false
        microphoneRunning = false
        aiRunning = false
    }

    func startCamera() -> Bool {
        cameraRunning = runtime.start(.camera)
        return cameraRunning
    }

    func stopCamera() {
        runtime.stop(.camera)
        cameraRunning = false
    }

    func startMicrophone() -> Bool {
        microphoneRunning = runtime.start(.microphone)
        return microphoneRunning
    }

    func stopMicrophone() {
        runtime.stop(.microphone)
        microphoneRunning = false
    }

    func startAI() -> Bool {
        aiRunning = runtime.start(.ai)
        return aiRunning
    }

    func stopAI() {
        runtime.stop(.ai)
        aiRunning = false
    }

    /// Hand tracking is intentionally gated by camera consent.
    func startHandTracking() -> Bool {
        guard runtime.isEnabled(.camera) else { return false }
        return runtime.start(.camera)
    }

    func stopHandTracking() {
        runtime.stop(.camera)
        cameraRunning = false
    }
}
