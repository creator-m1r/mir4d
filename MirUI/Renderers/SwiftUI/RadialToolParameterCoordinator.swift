import AppKit
import SwiftUI

/// Presents the parameter HUD next to the pointer after a parameterized
/// radial tool is chosen.
///
/// The coordinator deliberately stays outside CADCommandRegistry.
/// Command execution remains on the existing command path.
///
/// Swift 6 concurrency rule:
/// Notification objects and NSEvent objects never cross an actor boundary.
/// Only small Sendable value snapshots are passed into MainActor tasks.
@MainActor
final class RadialToolParameterCoordinator {

    // MARK: - Shared

    static let shared = RadialToolParameterCoordinator()

    // MARK: - Sendable event snapshot

    private struct LocalEventInput: Sendable {

        enum Kind: Sendable {
            case mouseMoved
            case scrollWheel
            case keyDown
            case other
        }

        let kind: Kind
        let scrollingDeltaY: Double
        let keyCode: UInt16
    }

    // MARK: - Dependencies

    private let parameterStore = RadialToolParameterStore.shared
    private let settingsStore = RadialToolParameterSettingsStore.shared

    // MARK: - Notification observers

    private var radialEndedObserver: NSObjectProtocol?
    private var cancelObserver: NSObjectProtocol?
    private var activatedObserver: NSObjectProtocol?
    private var applicationInactiveObserver: NSObjectProtocol?

    // MARK: - UI

    private var panel: NSPanel?
    private var localEventMonitor: Any?
    private var lastPointerLocation: NSPoint?

    // MARK: - Parameterized commands

    private let parameterizedCommands: Set<String> = [
        "feature.extrude",
        "model.extrude",
        "transform.move",
        "model.revolve",
        "measure.distance",
        "manufacturing.route",
        "manufacturing.submit"
    ]

    // MARK: - Lifecycle

    private init() {
        installObservers()
    }

    // MARK: - Notification observers

    private func installObservers() {

        let center = NotificationCenter.default

        // Radial menu committed.
        radialEndedObserver = center.addObserver(
            forName: .mir4DRadialMenuEnded,
            object: nil,
            queue: .main
        ) { [weak self] notification in

            // IMPORTANT:
            // Extract values from Notification BEFORE entering Task.
            let userInfo = notification.userInfo

            let commit = (userInfo?["commit"] as? Bool) ?? false
            let dx = userInfo?["dx"] as? Double
            let dy = userInfo?["dy"] as? Double

            guard commit,
                  let dx,
                  let dy
            else {
                return
            }

            Task { @MainActor [weak self] in
                guard let self else {
                    return
                }

                guard self.settingsStore.settings.enabled else {
                    return
                }

                self.startSelectedTool(
                    dx: dx,
                    dy: dy
                )
            }
        }

        // Radial parameter tool cancelled.
        cancelObserver = center.addObserver(
            forName: .mir4DRadialToolCancelled,
            object: nil,
            queue: .main
        ) { [weak self] _ in

            Task { @MainActor [weak self] in
                self?.closePanel()
            }
        }

        // Radial parameter tool activated.
        activatedObserver = center.addObserver(
            forName: .mir4DRadialToolActivated,
            object: nil,
            queue: .main
        ) { [weak self] _ in

            Task { @MainActor [weak self] in
                self?.closePanel()
            }
        }

        // Application resigned active state.
        applicationInactiveObserver = center.addObserver(
            forName: NSApplication.didResignActiveNotification,
            object: nil,
            queue: .main
        ) { [weak self] _ in

            Task { @MainActor [weak self] in
                self?.closePanel()
            }
        }
    }

    // MARK: - Notification observer cleanup

    /// Explicit lifecycle cleanup.
    ///
    /// We intentionally do not use `deinit` here because
    /// NSObjectProtocol is non-Sendable under Swift 6.
    func stop() {

        let center = NotificationCenter.default

        if let radialEndedObserver {
            center.removeObserver(radialEndedObserver)
            self.radialEndedObserver = nil
        }

        if let cancelObserver {
            center.removeObserver(cancelObserver)
            self.cancelObserver = nil
        }

        if let activatedObserver {
            center.removeObserver(activatedObserver)
            self.activatedObserver = nil
        }

        if let applicationInactiveObserver {
            center.removeObserver(applicationInactiveObserver)
            self.applicationInactiveObserver = nil
        }

        closePanel()
    }

    // MARK: - Radial tool selection

    private func startSelectedTool(
        dx: Double,
        dy: Double
    ) {

        let settings = RadialMenuSettingsStore.shared.settings

        let panels = RadialMenuGeometry.enabledPanels(
            settings
        )

        guard
            let panelIndex = RadialMenuGeometry.panelIndex(
                for: dx,
                dy: dy,
                settings: settings
            ),
            panels.indices.contains(panelIndex)
        else {
            return
        }

        let panel = panels[panelIndex]

        guard
            let toolIndex = RadialMenuGeometry.toolIndex(
                for: dx,
                dy: dy,
                panel: panel,
                settings: settings
            ),
            panel.tools.indices.contains(toolIndex)
        else {
            return
        }

        let command = panel.tools[toolIndex].command

        guard parameterizedCommands.contains(command) else {
            return
        }

        parameterStore.begin(
            command: command
        )

        showPanelNearPointer()
    }

    // MARK: - Parameter panel

    private func showPanelNearPointer() {

        let mouse = NSEvent.mouseLocation

        lastPointerLocation = mouse

        let settings = settingsStore.settings

        let size = NSSize(
            width: settings.panelWidth,
            height: settings.panelHeight
        )

        let visibleScreen =
            NSScreen.screens.first {
                NSMouseInRect(
                    mouse,
                    $0.frame,
                    false
                )
            }
            ?? NSScreen.main

        let frame =
            visibleScreen?.visibleFrame
            ?? NSScreen.main?.visibleFrame
            ?? .zero

        let offset = settings.pointerOffset

        var origin = NSPoint(
            x: mouse.x + offset,
            y: mouse.y - size.height - offset
        )

        if origin.x + size.width > frame.maxX {
            origin.x =
                mouse.x - size.width - offset
        }

        if origin.y < frame.minY {
            origin.y =
                mouse.y + offset
        }

        let panel =
            self.panel
            ?? makePanel(size: size)

        panel.setFrame(
            NSRect(
                origin: origin,
                size: size
            ),
            display: true
        )

        panel.orderFrontRegardless()

        self.panel = panel

        installInputMonitorIfNeeded()
    }

    // MARK: - Local event monitor

    private func installInputMonitorIfNeeded() {

        guard localEventMonitor == nil else {
            return
        }

        localEventMonitor =
            NSEvent.addLocalMonitorForEvents(
                matching: [
                    .mouseMoved,
                    .scrollWheel,
                    .keyDown
                ]
            ) { [weak self] event in

                // NSEvent is non-Sendable.
                // Convert it immediately into a Sendable snapshot.
                let input =
                    Self.makeLocalEventInput(
                        from: event
                    )

                Task { @MainActor [weak self] in
                    self?.handleLocalEvent(input)
                }

                return event
            }
    }

    private static func makeLocalEventInput(
        from event: NSEvent
    ) -> LocalEventInput {

        let kind: LocalEventInput.Kind

        switch event.type {

        case .mouseMoved:
            kind = .mouseMoved

        case .scrollWheel:
            kind = .scrollWheel

        case .keyDown:
            kind = .keyDown

        default:
            kind = .other
        }

        return LocalEventInput(
            kind: kind,
            scrollingDeltaY: event.scrollingDeltaY,
            keyCode: event.keyCode
        )
    }

    // MARK: - Local event processing

    private func handleLocalEvent(
        _ input: LocalEventInput
    ) {

        guard parameterStore.active != nil else {
            return
        }

        let settings =
            settingsStore.settings

        switch input.kind {

        case .mouseMoved:

            guard let previous = lastPointerLocation else {
                lastPointerLocation =
                    NSEvent.mouseLocation
                return
            }

            let current =
                NSEvent.mouseLocation

            let delta =
                (current.y - previous.y)
                + (current.x - previous.x) * 0.35

            if abs(delta) > 0.05 {

                let step =
                    parameterStore.active?.valueStep
                    ?? 0.5

                let currentValue =
                    parameterStore.active?.value
                    ?? 0

                parameterStore.update(
                    value:
                        currentValue
                        + delta
                        * step
                        * settings.pointerSensitivity
                )
            }

            lastPointerLocation = current

        case .scrollWheel:

            let step =
                parameterStore.active?.valueStep
                ?? 0.5

            let currentValue =
                parameterStore.active?.value
                ?? 0

            parameterStore.update(
                value:
                    currentValue
                    + input.scrollingDeltaY
                    * step
                    * settings.scrollSensitivity
            )

        case .keyDown:

            if input.keyCode == 53,
               settings.escapeCancels {

                parameterStore.cancel()

            } else if input.keyCode == 36,
                      settings.returnCommits {

                parameterStore.commit()
            }

        case .other:
            break
        }
    }

    // MARK: - Panel creation

    private func makePanel(
        size: NSSize
    ) -> NSPanel {

        let panel = NSPanel(
            contentRect:
                NSRect(
                    origin: .zero,
                    size: size
                ),
            styleMask: [
                .borderless,
                .nonactivatingPanel
            ],
            backing: .buffered,
            defer: false
        )

        panel.level = .floating
        panel.isOpaque = false
        panel.backgroundColor = .clear
        panel.hasShadow = true
        panel.hidesOnDeactivate = false

        panel.collectionBehavior = [
            .canJoinAllSpaces,
            .fullScreenAuxiliary
        ]

        panel.isMovableByWindowBackground = false

        panel.contentView =
            NSHostingView(
                rootView:
                    RadialToolParameterOverlay(
                        store: parameterStore
                    )
            )

        return panel
    }

    // MARK: - Panel closing

    private func closePanel() {

        if let localEventMonitor {
            NSEvent.removeMonitor(
                localEventMonitor
            )

            self.localEventMonitor = nil
        }

        lastPointerLocation = nil

        panel?.orderOut(nil)
        panel = nil
    }
}

// MARK: - Bootstrap

@MainActor
private let mir4DRadialToolParameterCoordinatorBootstrap =
    RadialToolParameterCoordinator.shared
