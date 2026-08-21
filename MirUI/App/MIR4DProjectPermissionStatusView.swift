import SwiftUI

/// Compact permission status used by the project launcher.
struct MIR4DProjectPermissionStatusView: View {
    @ObservedObject var permissions: MIR4DProjectPermissions
    @ObservedObject var runtime: MIR4DOptionalModuleRuntime

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            Text("Разрешения при запуске")
                .font(.headline)

            permissionRow(
                title: "Камера",
                detail: cameraDetail,
                enabled: permissions.cameraEnabled,
                running: runtime.runningModules.contains(.camera)
            ) {
                permissions.cameraEnabled.toggle()
            }

            permissionRow(
                title: "Микрофон",
                detail: microphoneDetail,
                enabled: permissions.microphoneEnabled,
                running: runtime.runningModules.contains(.microphone)
            ) {
                permissions.microphoneEnabled.toggle()
            }

            permissionRow(
                title: "Локальный ИИ",
                detail: aiDetail,
                enabled: permissions.aiEnabled,
                running: runtime.runningModules.contains(.ai)
            ) {
                permissions.aiEnabled.toggle()
            }
        }
        .padding(14)
        .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 16))
    }

    private var cameraDetail: String {
        permissions.cameraEnabled ? "Готова к запуску • Hand Tracking" : "Отключена • камера не запускается"
    }

    private var microphoneDetail: String {
        permissions.microphoneEnabled ? "Готов к запуску • голосовое управление" : "Отключён • микрофон не запускается"
    }

    private var aiDetail: String {
        permissions.aiEnabled ? "Готов к запуску • локальные агенты" : "Отключён • AI-агенты не запускаются"
    }

    @ViewBuilder
    private func permissionRow(
        title: String,
        detail: String,
        enabled: Bool,
        running: Bool,
        action: @escaping () -> Void
    ) -> some View {
        Button(action: action) {
            HStack(spacing: 12) {
                Image(systemName: enabled ? "checkmark.circle.fill" : "circle")
                    .font(.title3)

                VStack(alignment: .leading, spacing: 2) {
                    Text(title)
                        .font(.body.weight(.semibold))
                    Text(detail)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                }

                Spacer()

                if running {
                    Label("Активно", systemImage: "bolt.fill")
                        .font(.caption.weight(.semibold))
                }
            }
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
        .accessibilityLabel(title)
        .accessibilityValue(enabled ? "разрешено" : "отключено")
    }
}
