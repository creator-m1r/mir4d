//
//  MIR4DStartupView.swift
//  MIR 4D
//

import SwiftUI
import AppKit

struct MIR4DStartupView: View {
    @EnvironmentObject private var appState: CADAppState
    @StateObject private var boot = MIR4DBootCoordinator()
    @State private var showStartMenu = false
    @State private var showWorkspace = false

    var body: some View {
        ZStack {
            if showWorkspace {
                CADMainView(appState: appState)
                    .transition(.opacity)
            } else {
                startupBackground

                switch boot.state {
                case .idle, .booting:
                    bootView
                case .ready, .warning:
                    if showStartMenu {
                        MIR4DStartMenuView(diagnostic: boot)
                            .transition(.opacity)
                    } else {
                        bootView
                    }
                case .failed:
                    bootFailureView
                }
            }
        }
        .frame(minWidth: 1280, minHeight: 800)
        .onAppear { startBoot() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in
            withAnimation(.easeInOut(duration: 0.35)) {
                showWorkspace = true
                showStartMenu = false
            }
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectClosed)) { _ in
            withAnimation(.easeInOut(duration: 0.35)) {
                showWorkspace = false
                showStartMenu = true
            }
        }
    }

    private var startupBackground: some View {
        ZStack {
            Color(red: 0.012, green: 0.018, blue: 0.030).ignoresSafeArea()
            RadialGradient(
                colors: [Color.white.opacity(0.035), Color.clear],
                center: .center,
                startRadius: 10,
                endRadius: 650
            )
            .ignoresSafeArea()
        }
    }

    private var bootView: some View {
        VStack(spacing: 28) {
            Spacer()
            Image(systemName: "cube.transparent")
                .font(.system(size: 68))
                .foregroundStyle(.white)
            VStack(spacing: 7) {
                Text("МИР 4D")
                    .font(.system(size: 42, weight: .bold, design: .rounded))
                    .foregroundStyle(.white)
                Text("Мечтай • Изобретай • Развивай")
                    .font(.system(size: 14))
                    .foregroundStyle(.secondary)
            }
            VStack(spacing: 10) {
                HStack {
                    Text(boot.currentTitle).foregroundStyle(.white)
                    Spacer()
                    Text("\(Int(boot.progress * 100))%")
                        .font(.system(size: 13, weight: .bold, design: .monospaced))
                        .foregroundStyle(.white)
                }
                ProgressView(value: boot.progress, total: 1)
            }
            .frame(maxWidth: 650)
            Text(boot.currentDetail)
                .font(.system(size: 12))
                .foregroundStyle(.secondary)
            bootSteps
            Spacer()
            Text("MIR 4D Engineering Platform")
                .font(.system(size: 11))
                .foregroundStyle(.secondary)
        }
        .padding(40)
    }

    private var bootSteps: some View {
        VStack(alignment: .leading, spacing: 7) {
            ForEach(boot.steps) { step in
                HStack(spacing: 9) {
                    Image(systemName: icon(for: step.severity)).font(.system(size: 11))
                    Text("\(step.index). \(step.title)")
                        .font(.system(size: 11, weight: .medium))
                    Spacer()
                    Text(step.detail)
                        .font(.system(size: 10, design: .monospaced))
                        .foregroundStyle(.secondary)
                }
                .foregroundStyle(foreground(for: step.severity))
            }
        }
        .frame(maxWidth: 650)
        .padding(18)
        .background(RoundedRectangle(cornerRadius: 12).fill(Color.white.opacity(0.025)))
    }

    private var bootFailureView: some View {
        VStack(spacing: 24) {
            Image(systemName: "exclamationmark.triangle")
                .font(.system(size: 58))
                .foregroundStyle(.red)
            Text("MIR 4D не может продолжить запуск")
                .font(.system(size: 25, weight: .semibold))
                .foregroundStyle(.white)
            Text("Обнаружены критические ошибки.")
                .foregroundStyle(.secondary)
            bootSteps
            HStack(spacing: 12) {
                Button("Повторить диагностику") {
                    boot.reset()
                    startBoot()
                }
                Button("Продолжить") {
                    showStartMenu = true
                }
                .buttonStyle(.borderedProminent)
            }
        }
        .padding(40)
    }

    private func startBoot() {
        guard boot.state == .idle else { return }
        Task {
            await boot.start()
            try? await Task.sleep(for: .milliseconds(450))
            guard boot.state == .ready || boot.state == .warning else { return }

            // Restore the last valid local project automatically. If none exists,
            // keep the normal start menu visible.
            if MIR4DProjectCommands.shared.restoreLastProject(appState: appState) {
                return
            }

            withAnimation(.easeInOut(duration: 0.5)) {
                showStartMenu = true
            }
        }
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
