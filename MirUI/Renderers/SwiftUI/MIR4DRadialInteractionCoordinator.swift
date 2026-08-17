import SwiftUI
import AppKit

/// Global interaction bridge for the immersive radial menu.
/// `]` is a hold gesture: press = open, trackpad motion = navigate, release = commit.
/// While held, the directional vector is magnetically stabilized so the selection feels
/// like a physical radial instrument rather than a collection of buttons.
@MainActor
final class MIR4DRadialInteractionCoordinator: ObservableObject {
    static let shared = MIR4DRadialInteractionCoordinator()

    private var monitor: Any?
    private var active = false
    private var rawVector = CGVector.zero
    private var vector = CGVector.zero
    private var lockedSector: Int?

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
                self.rawVector.dx += event.scrollingDeltaX
                self.rawVector.dy += event.scrollingDeltaY
                self.updateMagneticVector()
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
        rawVector = .zero
        vector = .zero
        lockedSector = nil
        NotificationCenter.default.post(name: .mir4DRadialMenuBegan, object: nil)
    }

    private func updateMagneticVector() {
        let settings = RadialMenuSettingsStore.shared.settings
        let panels = RadialMenuGeometry.enabledPanels(settings)
        guard !panels.isEmpty else {
            vector = rawVector
            return
        }

        let distance = hypot(rawVector.dx, rawVector.dy)
        guard distance >= settings.deadZone else {
            vector = rawVector
            lockedSector = nil
            return
        }

        let sectorWidth = (Double.pi * 2) / Double(panels.count)
        let rawAngle = normalizedAngle(atan2(-rawVector.dy, rawVector.dx) + Double.pi / 2)
        let candidate = min(Int((rawAngle / sectorWidth).rounded(.down)), panels.count - 1)

        // Hysteresis prevents the highlight from flickering when the finger travels
        // near a sector boundary. A sector can only change after crossing its inner
        // boundary plus a small angular buffer.
        if let current = lockedSector, current != candidate {
            let currentCenter = -Double.pi / 2 + sectorWidth * Double(current)
            let delta = angularDistance(rawAngle, normalizedAngle(currentCenter))
            let switchThreshold = sectorWidth * 0.58
            if delta < switchThreshold {
                return applyMagnet(angle: normalizedAngle(currentCenter), distance: distance)
            }
        }

        lockedSector = candidate
        let targetAngle = normalizedAngle(-Double.pi / 2 + sectorWidth * Double(candidate))
        applyMagnet(angle: targetAngle, distance: distance)
    }

    private func applyMagnet(angle: Double, distance: Double) {
        let settings = RadialMenuSettingsStore.shared.settings
        let strength: Double
        if distance <= settings.deadZone {
            strength = 0
        } else {
            let normalizedDistance = min(max((distance - settings.deadZone) / 120.0, 0), 1)
            // Stronger magnetic pull as the gesture travels outward. Near the centre
            // the hand remains free; farther out the selection becomes stable.
            strength = 0.18 + normalizedDistance * 0.42
        }

        let rawAngle = atan2(-rawVector.dy, rawVector.dx) + Double.pi / 2
        let blended = shortestBlend(from: rawAngle, to: angle, amount: strength)
        vector = CGVector(
            dx: cos(blended - Double.pi / 2) * distance,
            dy: -sin(blended - Double.pi / 2) * distance
        )
    }

    private func normalizedAngle(_ angle: Double) -> Double {
        var value = angle.truncatingRemainder(dividingBy: Double.pi * 2)
        if value < 0 { value += Double.pi * 2 }
        return value
    }

    private func angularDistance(_ a: Double, _ b: Double) -> Double {
        let difference = abs(normalizedAngle(a) - normalizedAngle(b))
        return min(difference, Double.pi * 2 - difference)
    }

    private func shortestBlend(from: Double, to: Double, amount: Double) -> Double {
        let delta = atan2(sin(to - from), cos(to - from))
        return from + delta * amount
    }

    private func end(commit: Bool) {
        guard active else { return }
        NotificationCenter.default.post(
            name: .mir4DRadialMenuEnded,
            object: nil,
            userInfo: ["commit": commit, "dx": vector.dx, "dy": vector.dy]
        )
        active = false
        rawVector = .zero
        vector = .zero
        lockedSector = nil
    }
}
