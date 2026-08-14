import AppKit
import SwiftUI

/// Owns floating windows for dockable panels placed in `.floating` zone.
/// Panel windows follow the panel state: they are shown when the panel is
/// visible and placed in `.floating`, closed when the panel is re-docked,
/// hidden, or closed by the user.
@MainActor
final class FloatingPanelManager: NSObject, NSWindowDelegate {
    static let shared = FloatingPanelManager()

    private var windows: [CADPanel: NSPanel] = [:]
    private var appState: CADAppState?

    private override init() {
        super.init()
    }

    /// Reconciles open floating windows with the current panel state.
    func sync(appState: CADAppState) {
        self.appState = appState
        for panel in windows.keys {
            if !isFloatingActive(panel, in: appState) {
                close(panel)
            }
        }
        for panel in appState.visiblePanels where appState.panelPlacement(for: panel) == .floating {
            if windows[panel] == nil {
                show(panel, in: appState)
            }
        }
    }

    func closeAll() {
        for panel in windows.keys {
            close(panel)
        }
    }

    private func isFloatingActive(_ panel: CADPanel, in appState: CADAppState) -> Bool {
        appState.visiblePanels.contains(panel) && appState.panelPlacement(for: panel) == .floating
    }

    private func show(_ panel: CADPanel, in appState: CADAppState) {
        let title = appState.ui.language == .russian ? panel.titleRU : panel.titleEN
        let window = NSPanel(
            contentRect: NSRect(x: 0, y: 0, width: 360, height: 440),
            styleMask: [.titled, .closable, .resizable, .fullSizeContentView],
            backing: .buffered,
            defer: false
        )
        window.title = title
        window.isFloatingPanel = true
        window.level = .floating
        window.hidesOnDeactivate = false
        window.isMovableByWindowBackground = true
        window.titlebarAppearsTransparent = true
        window.titleVisibility = .hidden
        window.backgroundColor = .clear
        window.isOpaque = false
        window.minSize = NSSize(width: 300, height: 320)
        window.contentMinSize = NSSize(width: 300, height: 320)
        window.delegate = self

        let hosting = NSHostingController(rootView: CADPanelView(panel: panel, appState: appState))
        window.contentViewController = hosting
        window.contentView?.wantsLayer = true

        cascade(window, for: panel)
        window.makeKeyAndOrderFront(nil)
        windows[panel] = window
    }

    private func close(_ panel: CADPanel) {
        guard let window = windows.removeValue(forKey: panel) else { return }
        window.delegate = nil
        window.close()
    }

    private func cascade(_ window: NSPanel, for panel: CADPanel) {
        if let main = NSApp.mainWindow {
            let base = main.frame.origin
            let offset = CGFloat(windows.count - 1) * 26
            window.setFrameOrigin(NSPoint(x: base.x + 40 + offset, y: base.y + 40 - offset))
        } else if let screen = window.screen ?? NSScreen.main {
            let visible = screen.visibleFrame
            let index = CGFloat(windows.count)
            window.setFrameOrigin(NSPoint(x: visible.maxX - 380 - index * 26, y: visible.maxY - 460 - index * 26))
        }
    }

    // MARK: - NSWindowDelegate

    func windowWillClose(_ notification: Notification) {
        guard let window = notification.object as? NSWindow else { return }
        guard let panel = windows.first(where: { $0.value === window })?.key else { return }
        windows.removeValue(forKey: panel)
        if let appState {
            appState.togglePanel(panel)
        }
    }
}