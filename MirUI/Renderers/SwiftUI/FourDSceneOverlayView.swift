import SwiftUI

struct FourDSceneOverlayView: View {
    @ObservedObject var appState: CADAppState

    var body: some View {
        VStack(alignment: .trailing, spacing: 6) {
            chip(icon: "clock.arrow.circlepath", text: String(format: "T %.3f s", appState.currentTime), color: MirTheme.Colors.time)
            chip(icon: "square.stack.3d.up", text: appState.timeState.scenarioID, color: MirTheme.Colors.accentBright)
            chip(icon: "arrow.triangle.branch", text: appState.timeState.branchID, color: MirTheme.Colors.event)

            if appState.workbench == .simulation || appState.workbench == .fourD {
                chip(
                    icon: "waveform.path.ecg",
                    text: localizedPhysics,
                    color: MirTheme.Colors.simulation
                )
            }

            if appState.isPlaying {
                HStack(spacing: 6) {
                    Circle()
                        .fill(MirTheme.Colors.time)
                        .frame(width: 6, height: 6)
                    Text(appState.ui.language == .russian ? "Воспроизведение" : "Playing")
                }
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.time)
                .padding(.horizontal, 9)
                .padding(.vertical, 6)
                .background(MirTheme.Colors.time.opacity(0.10))
                .clipShape(Capsule())
            }
        }
        .padding(12)
    }

    private var localizedPhysics: String {
        appState.ui.language == .russian
            ? appState.simulation.physics.titleRU
            : appState.simulation.physics.titleEN
    }

    private func chip(icon: String, text: String, color: Color) -> some View {
        HStack(spacing: 6) {
            Image(systemName: icon)
            Text(text)
                .lineLimit(1)
        }
        .font(MirTheme.Typography.caption)
        .foregroundStyle(MirTheme.Colors.textPrimary)
        .padding(.horizontal, 9)
        .padding(.vertical, 6)
        .background(color.opacity(0.10))
        .clipShape(Capsule())
        .overlay(Capsule().stroke(color.opacity(0.18), lineWidth: 1))
    }
}
