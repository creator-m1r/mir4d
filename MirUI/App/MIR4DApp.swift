//
//  MIR4DApp.swift
//  MIR 4D
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

/// Native macOS window configuration.
/// The green traffic-light uses Apple's fullscreen control and therefore shows
/// the standard enter/exit fullscreen arrows rather than the ordinary zoom icon.
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

        window.styleMask.formUnion([.titled, .closable, .miniaturizable, .resizable, .fullScreen])
        window.titleVisibility = .hidden
        window.titlebarAppearsTransparent = true
        window.isMovableByWindowBackground = true
        window.level = .floating
        window.collectionBehavior.insert(.fullScreenAuxiliary)

        // Prefer Apple's dedicated fullscreen button. If the current window
        // style does not expose it, fall back to the standard zoom button and
        // explicitly route it to native fullscreen.
        if let fullscreen = window.standardWindowButton(.fullScreenButton) {
            fullscreen.isHidden = false
            fullscreen.isEnabled = true
            fullscreen.target = window
            fullscreen.action = #selector(NSWindow.toggleFullScreen(_:))
            fullscreen.toolTip = "На весь экран"
        } else if let zoom = window.standardWindowButton(.zoomButton) {
            zoom.isHidden = false
            zoom.isEnabled = true
            zoom.target = window
            zoom.action = #selector(NSWindow.toggleFullScreen(_:))
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
        window.setFrame(screen.visibleFrame, display: true, animate: false)
        window.collectionBehavior.insert(.fullScreenAuxiliary)
    }
}
