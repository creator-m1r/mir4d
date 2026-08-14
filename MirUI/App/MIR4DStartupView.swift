//
//  MIR4DStartupView.swift
//  MIR 4D
//

import SwiftUI
import AppKit

struct MIR4DStartupView: View {
    @EnvironmentObject private var appState: CADAppState
    @EnvironmentObject private var launch: MIR4DLaunchCoordinator
    @StateObject private var boot = MIR4DBootCoordinator()
    @State private var showStartMenu = false
    @State private var showWorkspace = false
    @State private var diagnosticsLeaving = false
    @State private var didResolveLaunch = false

    var body: some View {
        ZStack {
            if showWorkspace {
                CADMainView(appState: appState).transition(.opacity)
            } else {
                startupBackground
                if showStartMenu {
                    MIR4DStartMenuView(diagnostic: boot)
                        .transition(.asymmetric(insertion: .move(edge: .bottom).combined(with: .opacity), removal: .move(edge: .top).combined(with: .opacity)))
                } else {
                    bootView.transition(.opacity)
                }
            }
        }
        .frame(minWidth: 1280, minHeight: 800)
        .onAppear { startBoot() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in enterWorkspace() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DExternalProjectURL)) { notification in
            guard let url = notification.object as? URL else { return }
            openExternalProject(url)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DStartWorkspace)) { notification in
            if let rawValue = notification.userInfo?["workbench"] as? String, let workbench = CADWorkbench(rawValue: rawValue) { appState.selectWorkbench(workbench) }
            enterWorkspace()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectClosed)) { _ in
            withAnimation(.easeInOut(duration: 0.45)) { showWorkspace = false; showStartMenu = true }
        }
    }

    private func enterWorkspace() {
        withAnimation(.easeInOut(duration: 0.55)) { showWorkspace = true; showStartMenu = false }
    }

    private func openExternalProject(_ url: URL) {
        didResolveLaunch = true
        guard MIR4DProjectStore.shared.isValidPackage(at: url) else {
            appState.showNotification("Не удалось открыть проект: пакет .mir4d недействителен.", type: .warning)
            showStartMenuAnimated(); return
        }
        MIR4DProjectCommands.shared.open(appState: appState, url: url)
    }

    private var startupBackground: some View {
        ZStack {
            Color(red: 0.012, green: 0.018, blue: 0.030).ignoresSafeArea()
            RadialGradient(colors: [Color.white.opacity(0.035), Color.clear], center: .center, startRadius: 10, endRadius: 650).ignoresSafeArea()
        }
    }

    private var bootView: some View {
        GeometryReader { proxy in
            ZStack {
                // Fixed center: diagnostics never participate in the logo layout.
                brandLockup
                    .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .center)

                // Fixed bottom diagnostics region. Its position never changes as steps are added.
                VStack(spacing: 0) {
                    Spacer()
                    diagnosticLayer
                        .frame(maxWidth: 720)
                        .frame(maxWidth: .infinity)
                        .padding(.horizontal, 40)
                        .padding(.bottom, 46)
                        .opacity(diagnosticsLeaving ? 0 : 1)
                        .offset(y: diagnosticsLeaving ? -44 : 0)
                        .animation(.easeOut(duration: 0.75), value: diagnosticsLeaving)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
            .frame(width: proxy.size.width, height: proxy.size.height)
            .overlay(alignment: .bottom) {
                Text("MIR 4D Engineering Platform")
                    .font(.system(size: 11, weight: .medium))
                    .foregroundStyle(.white.opacity(0.32))
                    .padding(.bottom, 18)
                    .opacity(diagnosticsLeaving ? 0 : 1)
                    .animation(.easeOut(duration: 0.75), value: diagnosticsLeaving)
            }
        }
    }

    private var brandLockup: some View {
        VStack(spacing: 10) {
            Image(systemName: "cube.transparent").font(.system(size: 64, weight: .light)).foregroundStyle(.white)
            Text("МИР 4D").font(.system(size: 40, weight: .bold, design: .rounded)).foregroundStyle(.white)
            Text("Мечтай • Изобретай • Развивай").font(.system(size: 14, weight: .medium)).foregroundStyle(.white.opacity(0.55))
        }
    }

    private var diagnosticLayer: some View {
        VStack(spacing: 16) {
            HStack(alignment: .top, spacing: 16) {
                VStack(alignment: .leading, spacing: 4) {
                    Text(boot.currentTitle).font(.system(size: 14, weight: .semibold)).foregroundStyle(.white)
                    Text(boot.currentDetail).font(.system(size: 11)).foregroundStyle(.white.opacity(0.52)).lineLimit(2)
                }
                Spacer(minLength: 12)
                Text("\(Int(boot.progress * 100))%").font(.system(size: 13, weight: .bold, design: .monospaced)).foregroundStyle(.white)
            }
            ProgressView(value: boot.progress, total: 1).tint(.white).animation(.easeInOut(duration: 0.25), value: boot.progress)
            bootSteps
        }
        .padding(22)
        .background(RoundedRectangle(cornerRadius: 16).fill(Color.white.opacity(0.045)).overlay(RoundedRectangle(cornerRadius: 16).stroke(Color.white.opacity(0.10), lineWidth: 1)))
        .shadow(color: .black.opacity(0.22), radius: 24, y: 12)
    }

    private var bootSteps: some View {
        VStack(alignment: .leading, spacing: 7) {
            ForEach(boot.steps) { step in
                HStack(spacing: 9) {
                    Image(systemName: icon(for: step.severity)).font(.system(size: 11))
                    Text("\(step.index). \(step.title)").font(.system(size: 11, weight: .medium))
                    Spacer(minLength: 12)
                    Text(step.detail).font(.system(size: 10, design: .monospaced)).foregroundStyle(.white.opacity(0.45)).lineLimit(1)
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
            withAnimation(.easeOut(duration: 0.75)) { diagnosticsLeaving = true }
            try? await Task.sleep(for: .milliseconds(650))
            resolveLaunch()
        }
    }

    private func resolveLaunch() {
        guard !didResolveLaunch else { return }
        didResolveLaunch = true

        // The startup contract is explicit: after diagnostics the user always sees
        // the Project Hub and chooses what to work with. Automatic restoration is
        // intentionally not performed here; "Continue" remains an explicit choice.
        let intent = launch.resolveAfterBoot(autoOpenLastProject: false)

        switch intent {
        case .externalProject(let url):
            openExternalProject(url)
        case .restoreLast, .startMenu:
            showStartMenuAnimated()
        }
    }

    private func showStartMenuAnimated() { withAnimation(.easeInOut(duration: 0.55)) { showStartMenu = true } }

    private func icon(for severity: MIR4DBootCoordinator.Severity) -> String {
        switch severity { case .info: return "circle"; case .success: return "checkmark.circle.fill"; case .warning: return "exclamationmark.triangle.fill"; case .error: return "xmark.circle.fill" }
    }

    private func foreground(for severity: MIR4DBootCoordinator.Severity) -> Color {
        switch severity { case .info: return .secondary; case .success: return .green; case .warning: return .yellow; case .error: return .red }
    }
}
