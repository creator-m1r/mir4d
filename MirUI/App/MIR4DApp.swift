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
/// The green traffic-light uses the standard zoom button as the fullscreen action.
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
    /// SwiftUI layout pass is running (e.g. inside viewDidMoveToWindow called
    /// from addSubview). All window mutations are deferred to the next
    /// run-loop turn and performed exactly once.
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

        // Fullscreen must NOT be inserted into styleMask directly: macOS only
        // accepts it during a real toggleFullScreen transition. The zoom
        // button below is bound to toggleFullScreen and manages it itself.
        window.styleMask.formUnion([.titled, .closable, .miniaturizable, .resizable])
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

        DispatchQueue.main.async { [weak self, weak window] in
            guard let self, let window else { return }
            self.positionAlmostFullscreen(window)
            window.makeKeyAndOrderFront(nil)
            NSApp.activate(ignoringOtherApps: true)
        }
    }

    private func positionAlmostFullscreen(_ window: NSWindow) {
        guard let screen = window.screen ?? NSScreen.main else { return }
        let visible = screen.visibleFrame
        let width = visible.width * 0.92
        let height = visible.height * 0.92
        let origin = NSPoint(
            x: visible.midX - width / 2,
            y: visible.midY - height / 2
        )
        window.setFrame(
            NSRect(x: origin.x, y: origin.y, width: width, height: height),
            display: true,
            animate: false
        )
        window.collectionBehavior.insert(.fullScreenAuxiliary)
    }
}
