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
            MIR4DLaunchExperienceView()
                .environmentObject(appState)
                .environmentObject(launch)
                .frame(minWidth: 1280, minHeight: 800)
                .background(MIR4DWindowConfigurator())
                .onOpenURL { url in
                    launch.handleOpenURL(url)
                }
        }
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

            #if canImport(MirServer)
            MIR4DServerCommands()
            #endif
        }

        #if canImport(MirServer)
        Window("Сервер MIR 4D", id: "mir4d-server") {
            MIR4DTeamServerView()
        }

        Window("Совместная работа MIR 4D", id: "mir4d-collab") {
            MIR4DCollaborationView()
        }
        #endif
    }
}

/// Native macOS window configuration. The system title bar and traffic-light
/// controls remain owned by AppKit; MIR 4D renders only inside the content area.
struct MIR4DWindowConfigurator: NSViewRepresentable {
    func makeNSView(context: Context) -> WindowConfiguratorView { WindowConfiguratorView() }

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

    func scheduleConfiguration() {
        guard !didConfigure, window != nil else { return }
        didConfigure = true
        DispatchQueue.main.async { [weak self] in self?.configureWindow() }
    }

    private func configureWindow() {
        guard let window else { return }
        window.styleMask.formUnion([.titled, .closable, .miniaturizable, .resizable])
        window.titleVisibility = .visible
        window.titlebarAppearsTransparent = false
        window.isMovableByWindowBackground = false
        window.level = .normal
        window.collectionBehavior.remove(.fullScreenAuxiliary)
        window.title = "МИР 4D"

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
        let width = min(visible.width * 0.94, 1800)
        let height = min(visible.height * 0.92, 1100)
        let origin = NSPoint(x: visible.midX - width / 2, y: visible.midY - height / 2)
        window.setFrame(NSRect(x: origin.x, y: origin.y, width: width, height: height), display: true, animate: false)
    }
}
