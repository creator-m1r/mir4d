import SwiftUI
import AppKit
import MirUIHandGesture

/// Full launch choreography: diagnostics -> project hub -> immersive CAD workspace.
struct MIR4DLaunchExperienceView: View {
    @EnvironmentObject private var appState: CADAppState
    @EnvironmentObject private var launch: MIR4DLaunchCoordinator
    @EnvironmentObject private var boot: MIR4DBootCoordinator

    @State private var cardVisible = false
    @State private var diagnosticsLeaving = false
    @State private var workspaceVisible = false
    @State private var hubVisible = false
    @State private var hubLeaving = false

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                Color.black.ignoresSafeArea()

                if launch.phase == .workspace {
                    MIR4DCreativeWorkspaceView(appState: appState)
                        .mir4DRadialKeyboardTrigger()
                        .opacity((workspaceVisible || launch.phase == .workspace) ? 1 : 0)
                        .scaleEffect(workspaceVisible ? 1 : 1.015)
                        .animation(.easeOut(duration: 0.65), value: workspaceVisible)
                } else {
                    MIR4DStartupMotionLayer().opacity(launch.phase == .projectHub ? 0.16 : 0.35)
                }

                if launch.phase == .diagnostics {
                    diagnosticsCard
                        .frame(width: min(430, proxy.size.width * 0.34), height: min(700, proxy.size.height * 0.84))
                        .offset(x: cardVisible ? 0 : -proxy.size.width * 0.44)
                        .opacity(cardVisible ? 1 : 0)
                        .animation(.timingCurve(0.16, 0.82, 0.22, 1, duration: 0.90), value: cardVisible)
                }

                if launch.phase == .projectHub || (launch.phase == .workspace && hubLeaving) {
                    MIR4DLaunchProjectSelectionView(isLeaving: $hubLeaving)
                        .opacity(hubVisible ? 1 : 0)
                        .transition(.move(edge: .trailing))
                        .animation(.timingCurve(0.16, 0.82, 0.22, 1, duration: 0.80), value: hubVisible)
                }
            }
            .frame(width: proxy.size.width, height: proxy.size.height)
            .clipped()
        }
        .frame(minWidth: 1280, minHeight: 800)
        .onAppear {
            startBoot()
            if CommandLine.arguments.contains("--debug-cad") {
                Task { @MainActor in
                    // Wait until diagnostics complete so the workspace does not
                    // open on top of the still-running boot card.
                    while !launch.diagnosticsCompleted {
                        try? await Task.sleep(for: .milliseconds(100))
                    }
                    NotificationCenter.default.post(name: .mir4DStartWorkspace, object: nil, userInfo: ["workbench": "model"])
                }
            }
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in revealWorkspace() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DExternalProjectURL)) { note in
            guard let url = note.object as? URL else { return }
            openExternalProject(url)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DStartWorkspace)) { note in
            if let raw = note.userInfo?["workbench"] as? String, let workbench = CADWorkbench(rawValue: raw) { appState.selectWorkbench(workbench) }
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
                ScrollView {
                    VStack(alignment: .leading, spacing: 6) {
                        ForEach(boot.steps) { step in
                            HStack(spacing: 7) {
                                Image(systemName: diagnosticIcon(step.severity)).font(.system(size: 9)).foregroundStyle(diagnosticColor(step.severity)).frame(width: 12)
                                Text(step.title).font(.system(size: 9)).foregroundStyle(.white.opacity(0.66)).lineLimit(1)
                                Spacer(minLength: 4)
                                Text(step.detail).font(.system(size: 8, design: .monospaced)).foregroundStyle(.white.opacity(0.42)).lineLimit(1)
                            }
                        }
                    }
                    .frame(maxWidth: .infinity, alignment: .leading)
                }
                .frame(maxHeight: 230)
                Text(boot.currentTitle).font(.system(size: 9)).foregroundStyle(.white.opacity(0.40)).lineLimit(1)
                ProgressView(value: boot.progress, total: 1).tint(.blue).animation(.easeInOut(duration: 0.2), value: boot.progress)
                if boot.state == .ready || boot.state == .warning || boot.state == .failed {
                    HStack(spacing: 6) {
                        Circle().fill(bootStateColor).frame(width: 6, height: 6)
                        Text(bootStateLabel).font(.system(size: 9, weight: .medium, design: .monospaced)).foregroundStyle(bootStateColor)
                    }
                }
            }
            .padding(.horizontal, 32)
            .padding(.bottom, 34)
        }
        .background(RoundedRectangle(cornerRadius: 12).fill(LinearGradient(colors: [Color(red: 0.035, green: 0.055, blue: 0.085), Color(red: 0.008, green: 0.012, blue: 0.020)], startPoint: .topLeading, endPoint: .bottomTrailing)))
        .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.white.opacity(0.20), lineWidth: 1))
        .overlay(alignment: .trailing) { Rectangle().fill(Color.blue.opacity(0.80)).frame(width: 2).blur(radius: 4) }
        .shadow(color: .blue.opacity(0.13), radius: 35, x: 14, y: 0)
        .offset(x: diagnosticsLeaving ? -70 : 0)
        .opacity(diagnosticsLeaving ? 0.15 : 1)
        .animation(.easeInOut(duration: 0.62), value: diagnosticsLeaving)
    }

    private func diagnosticIcon(_ severity: MIR4DBootCoordinator.Severity) -> String {
        switch severity { case .success: return "checkmark.circle.fill"; case .warning: return "exclamationmark.circle.fill"; case .error: return "xmark.circle.fill"; case .info: return "circle" }
    }
    private func diagnosticColor(_ severity: MIR4DBootCoordinator.Severity) -> Color {
        switch severity { case .success: return .blue; case .warning: return .yellow; case .error: return .red; case .info: return .white.opacity(0.28) }
    }
    private var bootStateColor: Color {
        switch boot.state { case .failed: return .red; case .warning: return .yellow; case .ready: return .green; default: return .white.opacity(0.40) }
    }
    private var bootStateLabel: String {
        switch boot.state { case .ready: return "ГОТОВО"; case .warning: return "ГОТОВО С ПРЕДУПРЕЖДЕНИЯМИ"; case .failed: return "ОШИБКА ЗАПУСКА"; default: return "" }
    }

    private func startBoot() {
        // View may be recreated (e.g. after creating a project or returning
        // from the workspace). Never re-run diagnostics once they completed.
        guard !launch.diagnosticsCompleted else {
            if launch.phase == .diagnostics {
                launch.showProjectHub()
            }
            return
        }
        guard boot.state == .idle, !launch.launchResolved else { return }
        withAnimation(.easeOut(duration: 0.55)) { cardVisible = true }
        Task { @MainActor in
            await boot.start()
            try? await Task.sleep(for: .milliseconds(350))
            guard boot.state == .ready || boot.state == .warning else { return }
            launch.markBootFinished()
            launch.markDiagnosticsDone()
            withAnimation(.easeOut(duration: 0.55)) { diagnosticsLeaving = true }
            try? await Task.sleep(for: .milliseconds(520))
            resolveLaunch()
        }
    }

    private func resolveLaunch() {
        guard !launch.launchResolved else { return }
        launch.markLaunchResolved()
        switch launch.resolveAfterBoot(autoOpenLastProject: false) {
        case .externalProject(let url): openExternalProject(url)
        case .restoreLast, .startMenu: showProjectHub()
        }
    }

    private func showProjectHub() {
        launch.showProjectHub()
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
        // Start the hand-tracking subsystem so the Vertical Slice v0.1 grab
        // pipeline (intent → ray → pick → preview → commit) receives live
        // gestures. `startCamera()` is idempotent: it no-ops if already running.
        MIRHandGestureModule.shared.startCamera()
        // Switch phase immediately so the workspace is revealed without
        // rebuilding the launch experience through a diagnostics reset. The
        // hub keeps `hubLeaving == true` so its slide-out animates out.
        launch.revealWorkspace()
    }
}
