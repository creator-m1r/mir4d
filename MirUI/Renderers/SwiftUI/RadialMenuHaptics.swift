import AppKit
import Foundation

/// Haptic feedback for the radial gesture.
///
/// The haptic layer is intentionally independent from:
/// - radial menu rendering;
/// - command registry;
/// - engineering state.
///
/// RadialMenuBegan / RadialMenuMoved / RadialMenuEnded
/// remain the single input path.
@MainActor
final class RadialMenuHaptics {

    // MARK: - Shared instance

    static let shared = RadialMenuHaptics()

    // MARK: - Dependencies

    private let settings = RadialMenuSettingsStore.shared

    // MARK: - Runtime state

    private var lastPanelIndex: Int?
    private var lastToolIndex: Int?

    // MARK: - Notification observers

    private var beganObserver: NSObjectProtocol?
    private var movedObserver: NSObjectProtocol?
    private var endedObserver: NSObjectProtocol?

    // MARK: - Lifecycle

    private init() {
        installObservers()
    }

    // MARK: - Observer installation

    private func installObservers() {

        let center = NotificationCenter.default

        beganObserver = center.addObserver(
            forName: .mir4DRadialMenuBegan,
            object: nil,
            queue: .main
        ) { [weak self] _ in

            Task { @MainActor [weak self] in
                self?.reset()
            }
        }

        movedObserver = center.addObserver(
            forName: .mir4DRadialMenuMoved,
            object: nil,
            queue: .main
        ) { [weak self] notification in

            // Extract Sendable values before crossing
            // the concurrency boundary.
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

    // MARK: - Observer removal

    /// Removes NotificationCenter observers.
    ///
    /// This is deliberately an explicit lifecycle operation instead
    /// of using `deinit`, because NSObjectProtocol is non-Sendable
    /// under Swift 6 strict concurrency.
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

    // MARK: - Gesture state

    private func reset() {
        lastPanelIndex = nil
        lastToolIndex = nil
    }

    // MARK: - Gesture processing

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

        // Entered another panel.
        if panelIndex != lastPanelIndex {
            lastPanelIndex = panelIndex
            lastToolIndex = toolIndex

            perform(.generic)
            return
        }

        // Entered another tool inside the same panel.
        if toolIndex != lastToolIndex {
            lastToolIndex = toolIndex

            perform(.alignment)
        }
    }

    // MARK: - Haptic output

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

// MARK: - Bootstrap

@MainActor
private let mir4DRadialMenuHapticsBootstrap =
    RadialMenuHaptics.shared
