//
//  MIR4DApp.swift
//  MIR 4D
//

import SwiftUI
import AppKit

@main
struct MIR4DApp: App {
    @StateObject private var appState = CADAppState()
    @StateObject private var launch = MIR4DLaunchCoordinator.shared

    var body: some Scene {
        WindowGroup {
            MIR4DStartupView()
                .environmentObject(appState)
                .environmentObject(launch)
                .frame(minWidth: 1280, minHeight: 800)
                .background(MIR4DWindowConfigurator())
                .onOpenURL { url in
                    launch.handleOpenURL(url)
                }
        }
        // Keep the native macOS title bar and traffic-light controls owned by
        // the window. MIR 4D's application header is rendered inside the
        // SwiftUI content area below it, never over the system title region.
        .windowStyle(.titleBar)
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
///
/// The window itself owns its title bar, traffic-light controls and fullscreen
/// transition. SwiftUI renders the MIR 4D application header inside the content
/// area, so no application control is positioned over the native title region.
struct MIR4DWindowConfigurator: NSViewRepresentable {
    func makeNSView(context: Context) -> WindowConfiguratorView {
        WindowConfiguratorView()
    }

    func updateNSView(_ nsView: WindowConfiguratorView, context: Context) {
        nsView.scheduleConfiguration()
    }
}

final class WindowConfiguratorView: NSView {
    private var didConfigure = false

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        scheduleConfiguration()
    }

    override func layout() {
        super.layout()
        scheduleConfiguration()
    }

    /// macOS forbids mutating the window styleMask synchronously while the
    /// SwiftUI layout pass is running. Window mutations are therefore deferred
    /// to the next run-loop turn and performed once.
    func scheduleConfiguration() {
        guard !didConfigure else { return }
        guard window != nil else { return }
        didConfigure = true

        DispatchQueue.main.async { [weak self] in
            self?.configureWindow()
        }
    }

    private func configureWindow() {
        guard let window else { return }

        // Keep the standard native title bar. The application UI must not
        // replace, cover or float over this system-owned region.
        window.styleMask.formUnion([.titled, .closable, .miniaturizable, .resizable])
        window.titleVisibility = .visible
        window.titlebarAppearsTransparent = false
        window.isMovableByWindowBackground = false
        window.level = .normal
        window.collectionBehavior.remove(.fullScreenAuxiliary)
        window.title = "МИР 4D"

        // CAD sessions must start from a single clean window: macOS window
        // restoration would replay every previously opened project window on
        // top of the fresh one, and LaunchServices would still deliver the
        // requested project URL separately.
        window.isRestorable = false

        if let zoom = window.standardWindowButton(.zoomButton) {
            zoom.isHidden = false
            zoom.isEnabled = true
            zoom.target = window
            zoom.action = #selector(NSWindow.toggleFullScreen(_:))
            zoom.toolTip = "На весь экран"
        }

        DispatchQueue.main.async { [weak self, weak window] in
            guard let self, let window else { return }
            self.positionInitialWindow(window)
            window.makeKeyAndOrderFront(nil)
            NSApp.activate(ignoringOtherApps: true)
        }
    }

    private func positionInitialWindow(_ window: NSWindow) {
        guard let screen = window.screen ?? NSScreen.main else { return }
        let visible = screen.visibleFrame

        // Start large enough for the full CAD shell while retaining a small
        // margin around the native window. The user can resize/maximize it.
        let width = min(visible.width * 0.94, 1800)
        let height = min(visible.height * 0.92, 1100)
        let origin = NSPoint(
            x: visible.midX - width / 2,
            y: visible.midY - height / 2
        )

        window.setFrame(
            NSRect(x: origin.x, y: origin.y, width: width, height: height),
            display: true,
            animate: false
        )
    }
}
