import SwiftUI
import AppKit

extension Notification.Name {
    static let mir4DSpatialMenuBegan = Notification.Name("MIR4D.SpatialMenuBegan")
    static let mir4DSpatialMenuMoved = Notification.Name("MIR4D.SpatialMenuMoved")
    static let mir4DSpatialMenuEnded = Notification.Name("MIR4D.SpatialMenuEnded")
}

@MainActor
final class MIRSpatialMenuGesture: ObservableObject {
    static let shared = MIRSpatialMenuGesture()

    private var monitor: Any?
    private var active = false
    private var rawVector = CGVector.zero
    private var stabilizedVector = CGVector.zero
    private var beganVia = ""

    private init() {}

    func start() {
        guard monitor == nil else { return }

        MIR4DRadialInteractionCoordinator.shared.stop()
        MIR4DRadialKeyboardTrigger.shared.stop()

        let settings = MIRSpatialMenuSettingsStore.shared.settings

        monitor = NSEvent.addLocalMonitorForEvents(matching: [.keyDown, .keyUp, .scrollWheel, .otherMouseDown, .otherMouseUp, .otherMouseDragged]) { [weak self] event in
            guard let self else { return event }

            if event.type == .keyDown,
               settings.keyboardTriggerEnabled,
               event.charactersIgnoringModifiers == "]",
               !event.isARepeat {
                self.begin(via: "keyboard")
                return nil
            }

            if event.type == .keyUp,
               settings.keyboardTriggerEnabled,
               event.charactersIgnoringModifiers == "]" {
                self.end(commit: true)
                return nil
            }

            if event.type == .scrollWheel {
                if self.active, settings.trackpadTriggerEnabled {
                    self.rawVector.dx += Double(event.scrollingDeltaX)
                    self.rawVector.dy += Double(event.scrollingDeltaY)
                    self.publishMove()
                    return nil
                }
                return event
            }

            if event.type == .otherMouseDown,
               settings.middleMouseTriggerEnabled,
               event.buttonNumber == 2 {
                self.begin(via: "middleMouse")
                return nil
            }

            if event.type == .otherMouseDragged,
               settings.middleMouseTriggerEnabled,
               event.buttonNumber == 2 {
                if self.active {
                    self.rawVector.dx += Double(event.deltaX)
                    self.rawVector.dy += Double(event.deltaY)
                    self.publishMove()
                }
                return nil
            }

            if event.type == .otherMouseUp,
               settings.middleMouseTriggerEnabled,
               event.buttonNumber == 2 {
                if self.active {
                    self.end(commit: true)
                }
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

    func injectBegan() {
        begin(via: "external")
    }

    func injectMoved(dx: Double, dy: Double) {
        guard active else { return }
        rawVector.dx += dx
        rawVector.dy += dy
        publishMove()
    }

    func injectEnded(commit: Bool) {
        guard active else { return }
        end(commit: commit)
    }

    private func begin(via: String) {
        guard !active else { return }
        active = true
        beganVia = via
        rawVector = .zero
        stabilizedVector = .zero
        NotificationCenter.default.post(
            name: .mir4DSpatialMenuBegan,
            object: nil,
            userInfo: ["via": via, "dx": 0.0, "dy": 0.0]
        )
    }

    private func publishMove() {
        let settings = MIRSpatialMenuSettingsStore.shared.settings
        stabilizedVector = stabilized(rawVector, settings: settings)
        NotificationCenter.default.post(
            name: .mir4DSpatialMenuMoved,
            object: nil,
            userInfo: [
                "dx": Double(stabilizedVector.dx),
                "dy": Double(stabilizedVector.dy),
                "via": beganVia
            ]
        )
    }

    private func end(commit: Bool) {
        guard active else { return }
        NotificationCenter.default.post(
            name: .mir4DSpatialMenuEnded,
            object: nil,
            userInfo: ["commit": commit, "dx": stabilizedVector.dx, "dy": stabilizedVector.dy, "via": beganVia]
        )
        active = false
        rawVector = .zero
        stabilizedVector = .zero
        beganVia = ""
    }

    private func stabilized(_ raw: CGVector, settings: MIRSpatialMenuSettings) -> CGVector {
        let distance = hypot(raw.dx, raw.dy)
        guard distance >= settings.deadZone else { return raw }

        let rawAngle = atan2(raw.dy, raw.dx)
        let tree = MIRSpatialMenuContext.tree(for: MIRSpatialMenuContextResolved.current)
        let count = max(1, min(tree.count, settings.maxLevel1Segments))
        let strength = 0.16 + min(max((distance - settings.deadZone) / 220.0, 0), 1) * 0.44
        let blended = MIRSpatialMenuLayout.magneticAngle(raw: rawAngle, count: count, strength: strength)

        let normalized = MIRSpatialMenuLayout.shortestSignedAngle(from: rawAngle, to: blended)
        let finalAngle = rawAngle + normalized * min(max(strength, 0), 1)
        return CGVector(dx: cos(finalAngle) * distance, dy: sin(finalAngle) * distance)
    }
}

@MainActor
enum MIRSpatialMenuContextResolved {
    static var current: MIRSpatialMenuSceneContext = .idle
}

#if os(iOS)
import UIKit

@MainActor
final class MIRSpatialMenuTouchActivation {
    private var workItem: DispatchWorkItem?

    func scheduleHold(delay: Double) {
        cancel()
        let item = DispatchWorkItem { [weak self] in
            self?.activate()
        }
        workItem = item
        DispatchQueue.main.asyncAfter(deadline: .now() + delay, execute: item)
    }

    func cancel() {
        workItem?.cancel()
        workItem = nil
    }

    private func activate() {
        workItem = nil
        MIRSpatialMenuGesture.shared.injectBegan()
    }
}
#endif