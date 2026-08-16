//
//  MIR4DStartupView.swift
//  MIR 4D
//
//  Startup choreography: diagnostics -> Project Hub -> workspace.
//  UI-only animation layer. MirEngine remains untouched.
//

import SwiftUI
import AppKit

struct MIR4DStartupView: View {
    @EnvironmentObject private var appState: CADAppState
    @EnvironmentObject private var launch: MIR4DLaunchCoordinator
    @StateObject private var boot = MIR4DBootCoordinator()

    private enum Phase { case diagnostics, projectHub, workspace }
    @State private var phase: Phase = .diagnostics
    @State private var diagnosticsLeaving = false
    @State private var hubEntering = false
    @State private var hubLeaving = false
    @State private var workspaceRevealing = false
    @State private var didResolveLaunch = false

    private let doorTiming = Animation.timingCurve(0.18, 0.80, 0.22, 1.0, duration: 0.88)
    private let revealTiming = Animation.timingCurve(0.16, 0.82, 0.22, 1.0, duration: 0.82)

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                workspace.zIndex(0)

                if phase != .workspace || hubLeaving {
                    darkness.zIndex(20).allowsHitTesting(false)
                }

                if phase == .diagnostics {
                    diagnosticDoor.zIndex(30)
                }

                if phase == .projectHub || hubLeaving {
                    projectHubDoor.zIndex(30)
                }
            }
            .frame(width: proxy.size.width, height: proxy.size.height)
            .clipped()
        }
        .frame(minWidth: 1280, minHeight: 800)
        .onAppear { startBoot() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in revealWorkspace() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DExternalProjectURL)) { notification in
            guard let url = notification.object as? URL else { return }
            openExternalProject(url)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DStartWorkspace)) { notification in
            if let rawValue = notification.userInfo?["workbench"] as? String,
               let workbench = CADWorkbench(rawValue: rawValue) { appState.selectWorkbench(workbench) }
            revealWorkspace()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectClosed)) { _ in returnToProjectHub() }
    }

    private var workspace: some View {
        CADMainView(appState: appState)
            .opacity(workspaceRevealing || phase == .workspace ? 1 : 0.001)
            .scaleEffect(workspaceRevealing || phase == .workspace ? 1 : 1.015)
            .animation(.easeOut(duration: 0.65), value: workspaceRevealing)
    }

    private var darkness: some View {
        Rectangle()
            .fill(Color(red: 0.006, green: 0.009, blue: 0.015))
            .ignoresSafeArea()
            .opacity(phase == .workspace && !hubLeaving ? 0 : 1)
            .animation(.easeOut(duration: 0.75), value: phase)
    }

    private var diagnosticDoor: some View {
        GeometryReader { proxy in
            ZStack {
                Color(red: 0.012, green: 0.018, blue: 0.030)
                MIR4DStartupMotionLayer()
                    .opacity(diagnosticsLeaving ? 0.0 : 1.0)
                    .animation(.easeOut(duration: 0.55), value: diagnosticsLeaving)

                MIR4DStartupBrandMark()
                    .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .center)
                    .opacity(diagnosticsLeaving ? 0 : 1)
                    .scaleEffect(diagnosticsLeaving ? 0.94 : 1)
                    .animation(.easeOut(duration: 0.55), value: diagnosticsLeaving)

                VStack(spacing: 0) {
                    Spacer()
                    diagnosticLayer
                        .frame(maxWidth: 760)
                        .frame(maxWidth: .infinity)
                        .padding(.horizontal, 40)
                        .padding(.bottom, 46)
                        .opacity(diagnosticsLeaving ? 0 : 1)
                        .offset(y: diagnosticsLeaving ? -58 : 0)
                        .animation(.easeOut(duration: 0.72), value: diagnosticsLeaving)
                }

                HStack {
                    Text("SELF-DIAGNOSTIC")
                    Spacer()
                    Text("MIR 4D ENGINEERING PLATFORM")
                }
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundStyle(.white.opacity(0.24))
                .padding(.horizontal, 22)
                .padding(.bottom, 18)
                .frame(maxHeight: .infinity, alignment: .bottom)
                .opacity(diagnosticsLeaving ? 0 : 1)
                .animation(.easeOut(duration: 0.45), value: diagnosticsLeaving)
            }
            .frame(width: proxy.size.width, height: proxy.size.height)
            .clipShape(Rectangle())
            .shadow(color: .black.opacity(0.50), radius: 38, x: 18, y: 0)
            .offset(x: diagnosticsLeaving ? -proxy.size.width : 0)
            .animation(
                diagnosticsLeaving
                    ? .timingCurve(0.20, 0.78, 0.22, 1.0, duration: 0.82)
                    : .timingCurve(0.20, 0.82, 0.25, 1.0, duration: 0.90),
                value: diagnosticsLeaving
            )
        }
        .transition(.identity)
    }

    private var projectHubDoor: some View {
        GeometryReader { proxy in
            ZStack {
                MIR4DStartupMotionLayer()
                    .opacity(0.22)
                    .allowsHitTesting(false)

                MIR4DStartMenuView(diagnostic: boot)
                    .frame(width: proxy.size.width, height: proxy.size.height)
                    .background(MirTheme.Colors.panel.opacity(0.15))
                    .clipShape(Rectangle())
                    .shadow(color: .black.opacity(0.42), radius: 34, x: -18, y: 0)
                    .offset(x: hubLeaving ? proxy.size.width : (hubEntering ? 0 : proxy.size.width))
                    .opacity(hubLeaving ? 0.72 : 1)
                    .animation(doorTiming, value: hubEntering)
                    .animation(revealTiming, value: hubLeaving)
            }
        }
        .onAppear {
            guard !hubLeaving else { return }
            withAnimation(doorTiming) { hubEntering = true }
        }
    }

    private var diagnosticLayer: some View {
        VStack(spacing: 16) {
            HStack(alignment: .top, spacing: 16) {
                VStack(alignment: .leading, spacing: 4) {
                    Text(boot.currentTitle)
                        .font(.system(size: 14, weight: .semibold))
                        .foregroundStyle(.white)
                    Text(boot.currentDetail)
                        .font(.system(size: 11))
                        .foregroundStyle(.white.opacity(0.52))
                        .lineLimit(2)
                }
                Spacer(minLength: 12)
                Text("\(Int(boot.progress * 100))%")
                    .font(.system(size: 13, weight: .bold, design: .monospaced))
                    .foregroundStyle(.white)
            }

            ProgressView(value: boot.progress, total: 1)
                .tint(.white)
                .animation(.easeInOut(duration: 0.25), value: boot.progress)

            bootSteps

            HStack(spacing: 14) {
                diagnosticBadge("GPU", status: boot.state == .failed ? "CHECK" : "OK")
                diagnosticBadge("EVENT BUS", status: "OK")
                diagnosticBadge("RESOURCES", status: "OK")
                Spacer()
                if boot.warningCount > 0 {
                    Label("\(boot.warningCount) warning", systemImage: "exclamationmark.triangle.fill")
                        .foregroundStyle(.yellow)
                }
            }
            .font(.system(size: 9, weight: .semibold, design: .monospaced))
        }
        .padding(22)
        .background(
            RoundedRectangle(cornerRadius: 16)
                .fill(Color.white.opacity(0.045))
                .overlay(RoundedRectangle(cornerRadius: 16).stroke(Color.white.opacity(0.10), lineWidth: 1))
        )
        .shadow(color: .black.opacity(0.22), radius: 24, y: 12)
    }

    private func diagnosticBadge(_ title: String, status: String) -> some View {
        HStack(spacing: 5) {
            Circle().fill(status == "OK" ? Color.green : Color.orange).frame(width: 5, height: 5)
            Text(title)
            Text(status).foregroundStyle(.white.opacity(0.38))
        }
    }

    private var bootSteps: some View {
        VStack(alignment: .leading, spacing: 7) {
            ForEach(boot.steps) { step in
                HStack(spacing: 9) {
                    Image(systemName: icon(for: step.severity)).font(.system(size: 11))
                    Text("\(step.index). \(step.title)").font(.system(size: 11, weight: .medium))
                    Spacer(minLength: 12)
                    Text(step.detail)
                        .font(.system(size: 10, design: .monospaced))
                        .foregroundStyle(.white.opacity(0.45))
                        .lineLimit(1)
                }
                .foregroundStyle(foreground(for: step.severity))
                .transition(.opacity.combined(with: .move(edge: .top)))
            }
        }
        .animation(.easeInOut(duration: 0.3), value: boot.steps.count)
    }

    private func startBoot() {
        guard boot.state == .idle, !didResolveLaunch else { return }
        Task { @MainActor in
            await boot.start()
            try? await Task.sleep(for: .milliseconds(350))
            guard boot.state == .ready || boot.state == .warning else { return }
            launch.markBootFinished()
            withAnimation(.easeOut(duration: 0.72)) { diagnosticsLeaving = true }
            try? await Task.sleep(for: .milliseconds(820))
            resolveLaunch()
        }
    }

    private func resolveLaunch() {
        guard !didResolveLaunch else { return }
        didResolveLaunch = true
        switch launch.resolveAfterBoot(autoOpenLastProject: false) {
        case .externalProject(let url): openExternalProject(url)
        case .restoreLast, .startMenu: showProjectHub()
        }
    }

    private func showProjectHub() {
        hubLeaving = false
        hubEntering = false
        phase = .projectHub
        withAnimation(doorTiming) { hubEntering = true }
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
        guard !hubLeaving else { return }
        if phase == .diagnostics { phase = .projectHub }
        withAnimation(revealTiming) {
            hubLeaving = true
            workspaceRevealing = true
        }
        Task { @MainActor in
            try? await Task.sleep(for: .milliseconds(760))
            phase = .workspace
            hubLeaving = false
        }
    }

    private func returnToProjectHub() {
        workspaceRevealing = false
        hubLeaving = false
        hubEntering = false
        phase = .projectHub
        withAnimation(doorTiming) { hubEntering = true }
    }

    private func icon(for severity: MIR4DBootCoordinator.Severity) -> String {
        switch severity {
        case .info: return "circle"
        case .success: return "checkmark.circle.fill"
        case .warning: return "exclamationmark.triangle.fill"
        case .error: return "xmark.circle.fill"
        }
    }

    private func foreground(for severity: MIR4DBootCoordinator.Severity) -> Color {
        switch severity {
        case .info: return .secondary
        case .success: return .green
        case .warning: return .yellow
        case .error: return .red
        }
    }
}
