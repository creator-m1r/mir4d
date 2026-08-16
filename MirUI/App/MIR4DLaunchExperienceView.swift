import SwiftUI
import AppKit

/// Full launch choreography based on the MIR 4D reference storyboard:
/// branded card -> diagnostics -> project door -> CAD workspace.
struct MIR4DLaunchExperienceView: View {
    @EnvironmentObject private var appState: CADAppState
    @EnvironmentObject private var launch: MIR4DLaunchCoordinator
    @StateObject private var boot = MIR4DBootCoordinator()

    private enum Phase { case diagnostics, projectHub, workspace }
    @State private var phase: Phase = .diagnostics
    @State private var cardVisible = false
    @State private var diagnosticsLeaving = false
    @State private var workspaceVisible = false
    @State private var launchResolved = false
    @State private var hubVisible = false
    @State private var hubLeaving = false

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                Color.black.ignoresSafeArea()

                if phase == .workspace {
                    CADMainView(appState: appState)
                        .opacity(workspaceVisible ? 1 : 0)
                        .scaleEffect(workspaceVisible ? 1 : 1.015)
                        .animation(.easeOut(duration: 0.65), value: workspaceVisible)
                } else {
                    MIR4DStartupMotionLayer().opacity(phase == .projectHub ? 0.16 : 0.35)
                }

                if phase == .diagnostics {
                    diagnosticsCard
                        .frame(width: min(430, proxy.size.width * 0.34), height: min(700, proxy.size.height * 0.84))
                        .offset(x: cardVisible ? 0 : -proxy.size.width * 0.44)
                        .opacity(cardVisible ? 1 : 0)
                        .animation(.timingCurve(0.16, 0.82, 0.22, 1, duration: 0.90), value: cardVisible)
                }

                if phase == .projectHub {
                    MIR4DLaunchProjectSelectionView(diagnostic: boot, isLeaving: $hubLeaving)
                        .opacity(hubVisible ? 1 : 0)
                        .transition(.move(edge: .trailing))
                        .animation(.timingCurve(0.16, 0.82, 0.22, 1, duration: 0.80), value: hubVisible)
                }
            }
            .frame(width: proxy.size.width, height: proxy.size.height)
            .clipped()
        }
        .frame(minWidth: 1280, minHeight: 800)
        .onAppear { startBoot() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in revealWorkspace() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DExternalProjectURL)) { note in
            guard let url = note.object as? URL else { return }
            openExternalProject(url)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DStartWorkspace)) { note in
            if let raw = note.userInfo?["workbench"] as? String,
               let workbench = CADWorkbench(rawValue: raw) {
                appState.selectWorkbench(workbench)
            }
            revealWorkspace()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectClosed)) { _ in
            hubLeaving = false
            showProjectHub()
        }
    }

    private var diagnosticsCard: some View {
        VStack(spacing: 0) {
            Spacer()
            VStack(spacing: 12) {
                MIR4DLaunchGeometricMark()
                Text("МИР 4D").font(.system(size: 38, weight: .medium, design: .rounded)).tracking(1.8).foregroundStyle(.white)
                Text("Мечтай · Изобретай · Развивай").font(.system(size: 12)).foregroundStyle(.white.opacity(0.52))
            }
            Spacer()

            VStack(alignment: .leading, spacing: 9) {
                HStack(spacing: 8) {
                    Circle().fill(.blue).frame(width: 6, height: 6)
                    Text("САМОИАГНОСТИКА").font(.system(size: 10, weight: .medium, design: .monospaced)).foregroundStyle(.white.opacity(0.55))
                    Spacer()
                    Text("\(Int(boot.progress * 100))%").font(.system(size: 10, design: .monospaced)).foregroundStyle(.white.opacity(0.65))
                }

                ForEach(Array(boot.steps.prefix(4))) { step in
                    HStack(spacing: 7) {
                        Image(systemName: diagnosticIcon(step.severity))
                            .font(.system(size: 9))
                            .foregroundStyle(diagnosticColor(step.severity))
                            .frame(width: 12)
                        Text(step.title)
                            .font(.system(size: 9))
                            .foregroundStyle(.white.opacity(0.66))
                            .lineLimit(1)
                        Spacer(minLength: 4)
                        Text(step.detail)
                            .font(.system(size: 8, design: .monospaced))
                            .foregroundStyle(.white.opacity(0.42))
                            .lineLimit(1)
                    }
                    .transition(.opacity.combined(with: .move(edge: .top)))
                }

                Text(boot.currentTitle)
                    .font(.system(size: 9))
                    .foregroundStyle(.white.opacity(0.40))
                    .lineLimit(1)

                ProgressView(value: boot.progress, total: 1)
                    .tint(.blue)
                    .animation(.easeInOut(duration: 0.2), value: boot.progress)
            }
            .padding(.horizontal, 32)
            .padding(.bottom, 34)
        }
        .background(RoundedRectangle(cornerRadius: 12).fill(
            LinearGradient(colors: [Color(red: 0.035, green: 0.055, blue: 0.085), Color(red: 0.008, green: 0.012, blue: 0.020)], startPoint: .topLeading, endPoint: .bottomTrailing)
        ))
        .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.white.opacity(0.20), lineWidth: 1))
        .overlay(alignment: .trailing) { Rectangle().fill(Color.blue.opacity(0.80)).frame(width: 2).blur(radius: 4) }
        .shadow(color: .blue.opacity(0.13), radius: 35, x: 14, y: 0)
        .offset(x: diagnosticsLeaving ? -70 : 0)
        .opacity(diagnosticsLeaving ? 0.15 : 1)
        .animation(.easeInOut(duration: 0.62), value: diagnosticsLeaving)
    }

    private func diagnosticIcon(_ severity: MIR4DBootCoordinator.Severity) -> String {
        switch severity {
        case .success: return "checkmark.circle.fill"
        case .warning: return "exclamationmark.circle.fill"
        case .error: return "xmark.circle.fill"
        case .info: return "circle"
        }
    }

    private func diagnosticColor(_ severity: MIR4DBootCoordinator.Severity) -> Color {
        switch severity {
        case .success: return .blue
        case .warning: return .yellow
        case .error: return .red
        case .info: return .white.opacity(0.28)
        }
    }

    private func startBoot() {
        guard boot.state == .idle, !launchResolved else { return }
        withAnimation(.easeOut(duration: 0.55)) { cardVisible = true }
        Task { @MainActor in
            await boot.start()
            try? await Task.sleep(for: .milliseconds(350))
            guard boot.state == .ready || boot.state == .warning else { return }
            launch.markBootFinished()
            withAnimation(.easeOut(duration: 0.55)) { diagnosticsLeaving = true }
            try? await Task.sleep(for: .milliseconds(520))
            resolveLaunch()
        }
    }

    private func resolveLaunch() {
        guard !launchResolved else { return }
        launchResolved = true
        switch launch.resolveAfterBoot(autoOpenLastProject: false) {
        case .externalProject(let url): openExternalProject(url)
        case .restoreLast, .startMenu: showProjectHub()
        }
    }

    private func showProjectHub() {
        phase = .projectHub
        hubLeaving = false
        hubVisible = false
        withAnimation(.timingCurve(0.16, 0.82, 0.22, 1, duration: 0.80)) { hubVisible = true }
    }

    private func openExternalProject(_ url: URL) {
        guard MIR4DProjectStore.shared.isValidPackage(at: url) else {
            appState.showNotification("Не удалось открыть проект: пакет .mir4d недействителен.", type: .warning)
            showProjectHub()
            return
        }
        MIR4DProjectCommands.shared.open(appState: appState, url: url)
    }

    private func revealWorkspace() {
        withAnimation(.timingCurve(0.18, 0.80, 0.22, 1, duration: 0.78)) {
            hubLeaving = true
            workspaceVisible = true
        }
        Task { @MainActor in
            try? await Task.sleep(for: .milliseconds(720))
            phase = .workspace
            hubLeaving = false
        }
    }
}
