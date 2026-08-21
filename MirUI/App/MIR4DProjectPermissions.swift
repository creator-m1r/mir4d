import Foundation
import SwiftUI

@MainActor
final class MIR4DProjectPermissions: ObservableObject {
    static let shared = MIR4DProjectPermissions()

    @Published var cameraEnabled: Bool { didSet { persist() } }
    @Published var microphoneEnabled: Bool { didSet { persist() } }
    @Published var aiEnabled: Bool { didSet { persist() } }

    private let defaults: UserDefaults

    private enum Keys {
        static let camera = "mir4d.permission.camera"
        static let microphone = "mir4d.permission.microphone"
        static let ai = "mir4d.permission.ai"
    }

    init(defaults: UserDefaults = .standard) {
        self.defaults = defaults
        cameraEnabled = defaults.bool(forKey: Keys.camera)
        microphoneEnabled = defaults.bool(forKey: Keys.microphone)
        aiEnabled = defaults.bool(forKey: Keys.ai)
    }

    var enabledModules: Set<MIR4DOptionalModule> {
        var result = Set<MIR4DOptionalModule>()
        if cameraEnabled { result.insert(.camera) }
        if microphoneEnabled { result.insert(.microphone) }
        if aiEnabled { result.insert(.ai) }
        return result
    }

    func reset() {
        cameraEnabled = false
        microphoneEnabled = false
        aiEnabled = false
    }

    func applyAtLaunch() {
        MIR4DOptionalModuleRuntime.shared.apply(enabledModules)
        NotificationCenter.default.post(
            name: .mir4DOptionalModulesConfigurationChanged,
            object: self,
            userInfo: [
                "cameraEnabled": cameraEnabled,
                "microphoneEnabled": microphoneEnabled,
                "aiEnabled": aiEnabled
            ]
        )
    }

    private func persist() {
        defaults.set(cameraEnabled, forKey: Keys.camera)
        defaults.set(microphoneEnabled, forKey: Keys.microphone)
        defaults.set(aiEnabled, forKey: Keys.ai)
    }
}

enum MIR4DOptionalModule: String, CaseIterable, Hashable, Sendable {
    case camera
    case microphone
    case ai
}

extension Notification.Name {
    static let mir4DOptionalModulesConfigurationChanged = Notification.Name("mir4DOptionalModulesConfigurationChanged")
}

struct MIR4DProjectPermissionsView: View {
    @ObservedObject var permissions: MIR4DProjectPermissions

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack {
                VStack(alignment: .leading, spacing: 3) {
                    Text("РАЗРЕШЕНИЯ МОДУЛЕЙ")
                        .font(.system(size: 10, weight: .bold, design: .rounded))
                        .tracking(1.2)
                        .foregroundStyle(.cyan.opacity(0.85))
                    Text("Если модуль не отмечен, он не запускается с проектом")
                        .font(.system(size: 10))
                        .foregroundStyle(.white.opacity(0.42))
                }
                Spacer()
                Image(systemName: "lock.shield")
                    .foregroundStyle(.white.opacity(0.35))
            }

            permissionRow(
                icon: "camera",
                title: "Камера",
                subtitle: "Распознавание рук и визуальные сенсоры",
                isOn: $permissions.cameraEnabled
            )

            permissionRow(
                icon: "mic",
                title: "Микрофон",
                subtitle: "Голосовое управление и аудио-ввод",
                isOn: $permissions.microphoneEnabled
            )

            permissionRow(
                icon: "brain",
                title: "Локальный ИИ",
                subtitle: "Локальные AI-агенты и интеллектуальные инструменты",
                isOn: $permissions.aiEnabled
            )
        }
        .padding(18)
        .background(Color.white.opacity(0.035), in: RoundedRectangle(cornerRadius: 16))
        .overlay(RoundedRectangle(cornerRadius: 16).stroke(Color.white.opacity(0.08), lineWidth: 1))
    }

    private func permissionRow(
        icon: String,
        title: String,
        subtitle: String,
        isOn: Binding<Bool>
    ) -> some View {
        Toggle(isOn: isOn) {
            HStack(spacing: 11) {
                Image(systemName: icon)
                    .font(.system(size: 14, weight: .semibold))
                    .foregroundStyle(isOn.wrappedValue ? .cyan : .white.opacity(0.38))
                    .frame(width: 30, height: 30)
                    .background(Color.white.opacity(0.05), in: RoundedRectangle(cornerRadius: 8))

                VStack(alignment: .leading, spacing: 2) {
                    Text(title)
                        .font(.system(size: 12, weight: .semibold))
                        .foregroundStyle(.white)
                    Text(subtitle)
                        .font(.system(size: 9))
                        .foregroundStyle(.white.opacity(0.40))
                }
            }
        }
        .toggleStyle(.checkbox)
        .tint(.cyan)
        .padding(.vertical, 2)
    }
}
