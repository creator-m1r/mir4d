import SwiftUI
import AppKit

/// Global interaction bridge for the immersive radial menu.
/// The `]` key is a hold gesture: press = open, trackpad motion = navigate, release = commit.
@MainActor
final class MIR4DRadialInteractionCoordinator: ObservableObject {
    static let shared = MIR4DRadialInteractionCoordinator()

    private var monitor: Any?
    private var active = false
    private var vector = CGVector.zero

    private init() {}

    func start() {
        guard monitor == nil else { return }

        monitor = NSEvent.addLocalMonitorForEvents(matching: [.keyDown, .keyUp, .scrollWheel]) { [weak self] event in
            guard let self else { return event }

            if event.type == .keyDown,
               event.charactersIgnoringModifiers == "]",
               !event.isARepeat {
                self.begin()
                return nil
            }

            if event.type == .keyUp,
               event.charactersIgnoringModifiers == "]" {
                self.end(commit: true)
                return nil
            }

            if event.type == .scrollWheel, self.active {
                self.vector.dx += event.scrollingDeltaX
                self.vector.dy += event.scrollingDeltaY
                NotificationCenter.default.post(
                    name: .mir4DRadialMenuMoved,
                    object: nil,
                    userInfo: ["dx": self.vector.dx, "dy": self.vector.dy]
                )
                return nil
            }

            return event
        }
    }

    func stop() {
        if let monitor {
            NSEvent.removeMonitor(monitor)
            self.monitor = nil
        }
        end(commit: false)
    }

    private func begin() {
        guard !active else { return }
        active = true
        vector = .zero
        NotificationCenter.default.post(name: .mir4DRadialMenuBegan, object: nil)
    }

    private func end(commit: Bool) {
        guard active else { return }
        NotificationCenter.default.post(
            name: .mir4DRadialMenuEnded,
            object: nil,
            userInfo: ["commit": commit, "dx": vector.dx, "dy": vector.dy]
        )
        active = false
        vector = .zero
    }
}
