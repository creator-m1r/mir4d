import SwiftUI
import AppKit

/// Global application-local trigger for the immersive radial menu.
/// The radial menu is intentionally centred on the usable display rather than
/// following the pointer. `]` is a hold gesture: key-down opens, key-up commits.
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
            self.beginAtCenter()
            return nil
        }

        keyUpMonitor = NSEvent.addLocalMonitorForEvents(matching: .keyUp) { [weak self] event in
            guard let self else { return event }
            guard event.charactersIgnoringModifiers == "]" else { return event }
            self.endAtCenter()
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

    private func beginAtCenter() {
        guard !active, RadialMenuSettingsStore.shared.settings.enabled else { return }
        active = true
        NotificationCenter.default.post(
            name: .mir4DRadialMenuBegan,
            object: nil,
            userInfo: ["x": 0.5, "y": 0.5, "dx": 0.0, "dy": 0.0]
        )
    }

    private func endAtCenter() {
        guard active else { return }
        active = false
        NotificationCenter.default.post(
            name: .mir4DRadialMenuEnded,
            object: nil,
            userInfo: ["commit": true, "x": 0.5, "y": 0.5, "dx": 0.0, "dy": 0.0]
        )
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
