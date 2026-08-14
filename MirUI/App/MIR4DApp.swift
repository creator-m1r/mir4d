//
//  MIR4DApp.swift
//  MIR4D
//
//  Главная точка входа MIR 4D.
//

import SwiftUI
import AppKit

@main
struct MIR4DApp: App {
    @StateObject private var appState = CADAppState()

    var body: some Scene {
        WindowGroup {
            MIR4DStartupView()
                .environmentObject(appState)
                .frame(minWidth: 1280, minHeight: 800)
                .background(MIR4DWindowConfigurator())
        }
        .windowStyle(.hiddenTitleBar)
        .commands {
            CommandGroup(replacing: .newItem) {
                Button("Новый проект") {
                    NotificationCenter.default.post(name: .mir4DRequestNewProject, object: nil)
                }
                .keyboardShortcut("n", modifiers: [.command])

                Button("Открыть проект…") {
                    NotificationCenter.default.post(name: .mir4DOpenProject, object: nil)
                }
                .keyboardShortcut("o", modifiers: [.command])
            }

            CommandGroup(after: .saveItem) {
                Button("Сохранить") {
                    MIR4DProjectCommands.shared.save(appState: appState)
                }
                .keyboardShortcut("s", modifiers: [.command])

                Button("Сохранить как…") {
                    MIR4DProjectCommands.shared.saveAs(appState: appState)
                }
                .keyboardShortcut("s", modifiers: [.command, .shift])

                Divider()

                Button("Закрыть проект") {
                    MIR4DProjectCommands.shared.close(appState: appState)
                }
                .keyboardShortcut("w", modifiers: [.command])
            }
        }
    }
}

/// Applies the MIR 4D macOS window policy.
/// The app starts above normal windows and occupies the complete visible
/// desktop area while keeping the macOS menu bar and Dock area available.
struct MIR4DWindowConfigurator: NSViewRepresentable {
    func makeNSView(context: Context) -> WindowConfiguratorView {
        WindowConfiguratorView()
    }

    func updateNSView(_ nsView: WindowConfiguratorView, context: Context) {
        nsView.configureWindow()
    }
}

final class WindowConfiguratorView: NSView {
    private var didConfigure = false

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        configureWindow()
    }

    override func layout() {
        super.layout()
        configureWindow()
    }

    func configureWindow() {
        guard let window else { return }

        window.level = .floating
        window.collectionBehavior.insert(.fullScreenAuxiliary)
        window.titleVisibility = .hidden
        window.titlebarAppearsTransparent = true
        window.isMovableByWindowBackground = true

        // Keep the native macOS traffic-light controls. The green zoom button
        // is the standard macOS fullscreen/maximize control and opens native
        // fullscreen when the user presses it.
        if let zoom = window.standardWindowButton(.zoomButton) {
            zoom.isHidden = false
            zoom.isEnabled = true
            zoom.toolTip = "На весь экран"
        }

        if !didConfigure {
            didConfigure = true
            DispatchQueue.main.async { [weak self, weak window] in
                guard let self, let window else { return }
                self.positionAlmostFullscreen(window)
                window.makeKeyAndOrderFront(nil)
                NSApp.activate(ignoringOtherApps: true)
            }
        }
    }

    private func positionAlmostFullscreen(_ window: NSWindow) {
        guard let screen = window.screen ?? NSScreen.main else { return }

        // visibleFrame excludes the macOS menu bar and the Dock. This gives
        // MIR 4D the largest safe window without hiding system UI.
        window.setFrame(screen.visibleFrame, display: true, animate: false)
        window.collectionBehavior.insert(.fullScreenAuxiliary)
    }
}
