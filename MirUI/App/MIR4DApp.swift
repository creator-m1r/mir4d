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

/// Configures the native macOS window while preserving Apple's traffic lights.
/// The green control is explicitly configured for native fullscreen mode.
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

        // HiddenTitleBar hides title text, but we explicitly retain all native
        // controls and the fullscreen capability.
        window.styleMask.insert([.titled, .closable, .miniaturizable, .resizable, .fullScreen])
        window.titleVisibility = .hidden
        window.titlebarAppearsTransparent = true
        window.isMovableByWindowBackground = true
        window.level = .floating
        window.collectionBehavior.insert(.fullScreenAuxiliary)

        if let zoom = window.standardWindowButton(.zoomButton) {
            zoom.isHidden = false
            zoom.isEnabled = true
            zoom.target = window
            zoom.action = #selector(NSWindow.toggleFullScreen(_:))
            zoom.toolTip = "На весь экран"
        }

        // On macOS versions that expose a dedicated fullscreen traffic-light,
        // use it. AppKit owns the icon and changes it to enter/exit arrows.
        if let fullscreen = window.standardWindowButton(.fullScreenButton) {
            fullscreen.isHidden = false
            fullscreen.isEnabled = true
            fullscreen.target = window
            fullscreen.action = #selector(NSWindow.toggleFullScreen(_:))
            fullscreen.toolTip = "На весь экран"
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
        // visibleFrame adapts to each display and excludes the macOS menu bar
        // and Dock, so the app starts almost fullscreen without hiding them.
        window.setFrame(screen.visibleFrame, display: true, animate: false)
        window.collectionBehavior.insert(.fullScreenAuxiliary)
    }
}
