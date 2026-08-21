import SwiftUI
import AppKit

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

        guard let candidate = RadialMenuGeometry.panelIndex(for: rawVector.dx, dy: rawVector.dy, settings: settings) else {
            vector = rawVector
            return
        }

        if let current = lockedSector, current != candidate {
            let currentCenter = RadialMenuGeometry.sectorCenter(current, panels.count)
            let delta = angularDistance(normalizedAngle(atan2(rawVector.dy, rawVector.dx)), currentCenter)
            let switchThreshold = (Double.pi * 2 / Double(panels.count)) * (0.5 + settings.magneticHysteresis)
            if delta < switchThreshold {
                applyMagnet(angle: currentCenter, distance: distance)
                return
            }
        }

        lockedSector = candidate
        applyMagnet(angle: RadialMenuGeometry.sectorCenter(candidate, panels.count), distance: distance)
    }

    private func applyMagnet(angle: Double, distance: Double) {
        let settings = RadialMenuSettingsStore.shared.settings
        let strength = RadialMenuGeometry.magneticStrength(forDistance: distance, settings: settings)
        let rawAngle = atan2(rawVector.dy, rawVector.dx)
        let blended = shortestBlend(from: rawAngle, to: angle, amount: strength)
        vector = CGVector(dx: cos(blended) * distance, dy: sin(blended) * distance)
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
