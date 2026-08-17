import SwiftUI
import AppKit

/// Global application-local trigger for the immersive radial menu.
/// The viewport remains responsible for mouse-wheel activation; this coordinator
/// adds the MacBook `]` shortcut without stealing ordinary keyboard input.
@MainActor
final class MIR4DRadialKeyboardTrigger: ObservableObject {
    static let shared = MIR4DRadialKeyboardTrigger()

    private var keyDownMonitor: Any?
    private var keyUpMonitor: Any?
    private var active = false

    private init() {}

    func start() {
        guard keyDownMonitor == nil else { return }

        keyDownMonitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) { [weak self] event in
            guard let self else { return event }
            guard event.charactersIgnoringModifiers == "]" else { return event }
            guard !event.isARepeat else { return nil }
            self.beginAtCursor()
            return nil
        }

        keyUpMonitor = NSEvent.addLocalMonitorForEvents(matching: .keyUp) { [weak self] event in
            guard let self else { return event }
            guard event.charactersIgnoringModifiers == "]" else { return event }
            self.endAtCursor()
            return nil
        }
    }

    func stop() {
        if let keyDownMonitor {
            NSEvent.removeMonitor(keyDownMonitor)
            self.keyDownMonitor = nil
        }
        if let keyUpMonitor {
            NSEvent.removeMonitor(keyUpMonitor)
            self.keyUpMonitor = nil
        }
        active = false
    }

    private func beginAtCursor() {
        guard !active, RadialMenuSettingsStore.shared.settings.enabled else { return }
        guard let payload = cursorPayload() else { return }
        active = true
        NotificationCenter.default.post(name: .mir4DRadialMenuBegan, object: nil, userInfo: payload)
    }

    private func endAtCursor() {
        guard active else { return }
        let payload = cursorPayload() ?? ["x": 0.5, "y": 0.5]
        active = false
        NotificationCenter.default.post(name: .mir4DRadialMenuEnded, object: nil, userInfo: [
            "commit": true,
            "dx": 0.0,
            "dy": 0.0,
            "x": payload["x"] ?? 0.5,
            "y": payload["y"] ?? 0.5
        ])
    }

    private func cursorPayload() -> [String: Double]? {
        guard let window = NSApp.keyWindow, let content = window.contentView else { return nil }
        let windowPoint = window.convertPoint(fromScreen: NSEvent.mouseLocation)
        let point = content.convert(windowPoint, from: nil)
        guard content.bounds.width > 0, content.bounds.height > 0 else { return nil }
        return [
            "x": Double(point.x / content.bounds.width),
            "y": Double(1.0 - point.y / content.bounds.height),
            "dx": 0.0,
            "dy": 0.0
        ]
    }
}

struct MIR4DRadialKeyboardTriggerInstaller: ViewModifier {
    func body(content: Content) -> some View {
        content
            .onAppear { MIR4DRadialKeyboardTrigger.shared.start() }
            .onDisappear { MIR4DRadialKeyboardTrigger.shared.stop() }
    }
}

extension View {
    func mir4DRadialKeyboardTrigger() -> some View {
        modifier(MIR4DRadialKeyboardTriggerInstaller())
    }
}
