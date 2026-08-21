import AppKit
import SwiftUI

@MainActor
final class FloatingPanelManager: NSObject, NSWindowDelegate {
    static let shared = FloatingPanelManager()

    private var windows: [CADPanel: NSPanel] = [:]
    private var appState: CADAppState?
    private var closingPanels: Set<CADPanel> = []

    private override init() { super.init() }

    func sync(appState: CADAppState) {
        self.appState = appState
        for panel in Array(windows.keys) {
            if !isFloatingActive(panel, in: appState) { close(panel, animated: true) }
        }
        for panel in appState.visiblePanels where appState.panelPlacement(for: panel) == .floating {
            if windows[panel] == nil && !closingPanels.contains(panel) { show(panel, in: appState) }
        }
    }

    func closeAll() {
        for panel in Array(windows.keys) { close(panel, animated: false) }
    }

    func applySizeToAll(width: Double, height: Double) {
        let size = NSSize(width: max(300, width), height: max(280, height))
        for window in windows.values { window.setContentSize(size) }
    }

    private func isFloatingActive(_ panel: CADPanel, in appState: CADAppState) -> Bool {
        appState.visiblePanels.contains(panel) && appState.panelPlacement(for: panel) == .floating
    }

    private func show(_ panel: CADPanel, in appState: CADAppState) {
        let title = appState.ui.language == .russian ? panel.titleRU : panel.titleEN
        let preferences = MIR4DWorkspaceCustomizationStore.shared
        let window = NSPanel(
            contentRect: NSRect(x: 0, y: 0, width: preferences.floatingWidth, height: preferences.floatingHeight),
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
        window.alphaValue = 0
        window.minSize = NSSize(width: 300, height: 280)
        window.contentMinSize = NSSize(width: 300, height: 280)
        window.delegate = self

        let hosting = NSHostingController(rootView: CADPanelView(panel: panel, appState: appState))
        window.contentViewController = hosting
        window.contentView?.wantsLayer = true

        let targetOrigin = cascadeOrigin(for: panel, window: window)
        let direction = motionDirection(for: panel, appState: appState)
        let startOrigin = NSPoint(x: targetOrigin.x + direction.dx * 34, y: targetOrigin.y + direction.dy * 34)
        window.setFrameOrigin(startOrigin)
        window.makeKeyAndOrderFront(nil)
        windows[panel] = window

        NSAnimationContext.runAnimationGroup { context in
            context.duration = 0.28
            context.timingFunction = CAMediaTimingFunction(name: .easeOut)
            window.animator().alphaValue = 1
            window.animator().setFrameOrigin(targetOrigin)
        }
    }

    private func close(_ panel: CADPanel, animated: Bool) {
        guard let window = windows[panel], !closingPanels.contains(panel) else { return }
        guard animated else {
            window.delegate = nil
            windows.removeValue(forKey: panel)
            window.orderOut(nil)
            window.close()
            return
        }
        closingPanels.insert(panel)
        let origin = window.frame.origin
        let direction = motionDirection(for: panel, appState: appState)
        let target = NSPoint(x: origin.x + direction.dx * 30, y: origin.y + direction.dy * 30)
        NSAnimationContext.runAnimationGroup({ context in
            context.duration = 0.22
            context.timingFunction = CAMediaTimingFunction(name: .easeIn)
            window.animator().alphaValue = 0
            window.animator().setFrameOrigin(target)
        }, completionHandler: { [weak self, weak window] in
            Task { @MainActor in
                guard let self, let window else { return }
                self.closingPanels.remove(panel)
                self.windows.removeValue(forKey: panel)
                window.delegate = nil
                window.orderOut(nil)
                window.close()
            }
        })
    }

    private func cascadeOrigin(for panel: CADPanel, window: NSPanel) -> NSPoint {
        if let main = NSApp.mainWindow {
            let base = main.frame.origin
            let offset = CGFloat(max(0, windows.count - 1)) * 26
            return NSPoint(x: base.x + 40 + offset, y: base.y + 40 - offset)
        }
        let visible = (window.screen ?? NSScreen.main)?.visibleFrame ?? NSRect(x: 0, y: 0, width: 1440, height: 900)
        let index = CGFloat(windows.count)
        return NSPoint(x: visible.maxX - 380 - index * 26, y: visible.maxY - 460 - index * 26)
    }

    private func motionDirection(for panel: CADPanel, appState: CADAppState?) -> CGVector {
        guard let appState else { return CGVector(dx: 1, dy: 0) }
        switch appState.panelPlacement(for: panel) {
        case .left: return CGVector(dx: -1, dy: 0)
        case .right: return CGVector(dx: 1, dy: 0)
        case .bottom: return CGVector(dx: 0, dy: -1)
        case .floating: return CGVector(dx: 0.65, dy: -0.35)
        }
    }

    func windowWillClose(_ notification: Notification) {
        guard let window = notification.object as? NSWindow else { return }
        guard let panel = windows.first(where: { $0.value === window })?.key else { return }
        windows.removeValue(forKey: panel)
        closingPanels.remove(panel)
        if let appState, !isFloatingActive(panel, in: appState) { appState.togglePanel(panel) }
    }
}
