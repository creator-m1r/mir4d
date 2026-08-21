import AppKit
import Foundation

@MainActor
final class RadialMenuHaptics {

    static let shared = RadialMenuHaptics()

    enum RadialMenuHapticEvent {
        case open
        case enterDeadZone
        case enterPanel
        case switchPanel
        case enterTool
        case switchTool
        case preview
        case confirm
        case cancel
        case error
    }

    func fire(_ event: RadialMenuHapticEvent) {
        guard settings.settings.hapticEnabled else { return }
        let pattern: NSHapticFeedbackManager.FeedbackPattern
        switch event {
        case .open: pattern = .alignment
        case .enterDeadZone: pattern = .alignment
        case .enterPanel: pattern = .alignment
        case .switchPanel: pattern = .alignment
        case .enterTool: pattern = .alignment
        case .switchTool: pattern = .alignment
        case .preview: pattern = .generic
        case .confirm: pattern = .generic
        case .cancel: pattern = .alignment
        case .error: pattern = .generic
        }
        perform(pattern)
    }

    private let settings = RadialMenuSettingsStore.shared

    private var lastPanelIndex: Int?
    private var lastToolIndex: Int?

    private var beganObserver: NSObjectProtocol?
    private var movedObserver: NSObjectProtocol?
    private var endedObserver: NSObjectProtocol?

    private init() {
        installObservers()
    }

    private func installObservers() {

        let center = NotificationCenter.default

        beganObserver = center.addObserver(
            forName: .mir4DRadialMenuBegan,
            object: nil,
            queue: .main
        ) { [weak self] _ in

            Task { @MainActor [weak self] in
                self?.fire(.open)
                self?.reset()
            }
        }

        movedObserver = center.addObserver(
            forName: .mir4DRadialMenuMoved,
            object: nil,
            queue: .main
        ) { [weak self] notification in

            guard
                let dx = notification.userInfo?["dx"] as? Double,
                let dy = notification.userInfo?["dy"] as? Double
            else {
                return
            }

            Task { @MainActor [weak self] in
                self?.handleMove(
                    dx: dx,
                    dy: dy
                )
            }
        }

        endedObserver = center.addObserver(
            forName: .mir4DRadialMenuEnded,
            object: nil,
            queue: .main
        ) { [weak self] _ in

            Task { @MainActor [weak self] in
                self?.reset()
            }
        }
    }

    func stop() {

        let center = NotificationCenter.default

        if let beganObserver {
            center.removeObserver(beganObserver)
            self.beganObserver = nil
        }

        if let movedObserver {
            center.removeObserver(movedObserver)
            self.movedObserver = nil
        }

        if let endedObserver {
            center.removeObserver(endedObserver)
            self.endedObserver = nil
        }

        reset()
    }

    private func reset() {
        lastPanelIndex = nil
        lastToolIndex = nil
    }

    private func handleMove(
        dx: Double,
        dy: Double
    ) {
        guard settings.settings.hapticEnabled else {
            return
        }

        let currentSettings = settings.settings

        let panels = RadialMenuGeometry.enabledPanels(
            currentSettings
        )

        guard !panels.isEmpty else {
            return
        }

        let panelIndex = RadialMenuGeometry.panelIndex(
            for: dx,
            dy: dy,
            settings: currentSettings
        )

        let toolIndex: Int? = {
            guard
                let panelIndex,
                panels.indices.contains(panelIndex)
            else {
                return nil
            }

            return RadialMenuGeometry.toolIndex(
                for: dx,
                dy: dy,
                panel: panels[panelIndex],
                settings: currentSettings
            )
        }()

        if panelIndex != lastPanelIndex {
            lastPanelIndex = panelIndex
            lastToolIndex = toolIndex

            fire(.switchPanel)
            return
        }

        if toolIndex != lastToolIndex {
            lastToolIndex = toolIndex

            fire(.switchTool)
        }
    }

    private func perform(
        _ pattern: NSHapticFeedbackManager.FeedbackPattern
    ) {
        NSHapticFeedbackManager
            .defaultPerformer
            .perform(
                pattern,
                performanceTime: .now
            )
    }
}

@MainActor
private let mir4DRadialMenuHapticsBootstrap =
    RadialMenuHaptics.shared
