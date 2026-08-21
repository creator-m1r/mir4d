import AppKit
import SwiftUI

@MainActor
final class RadialToolParameterCoordinator {

    static let shared = RadialToolParameterCoordinator()

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

    private let parameterStore = RadialToolParameterStore.shared
    private let settingsStore = RadialToolParameterSettingsStore.shared

    private var radialEndedObserver: NSObjectProtocol?
    private var cancelObserver: NSObjectProtocol?
    private var activatedObserver: NSObjectProtocol?
    private var applicationInactiveObserver: NSObjectProtocol?

    private var panel: NSPanel?
    private var localEventMonitor: Any?
    private var lastPointerLocation: NSPoint?

    private let parameterizedCommands: Set<String> = [
        "feature.extrude",
        "model.extrude",
        "transform.move",
        "model.revolve",
        "measure.distance",
        "manufacturing.route",
        "manufacturing.submit"
    ]

    private init() {
        installObservers()
    }

    private func installObservers() {

        let center = NotificationCenter.default

        radialEndedObserver = center.addObserver(
            forName: .mir4DRadialMenuEnded,
            object: nil,
            queue: .main
        ) { [weak self] notification in

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

        cancelObserver = center.addObserver(
            forName: .mir4DRadialToolCancelled,
            object: nil,
            queue: .main
        ) { [weak self] _ in

            Task { @MainActor [weak self] in
                self?.closePanel()
            }
        }

        activatedObserver = center.addObserver(
            forName: .mir4DRadialToolActivated,
            object: nil,
            queue: .main
        ) { [weak self] _ in

            Task { @MainActor [weak self] in
                self?.closePanel()
            }
        }

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

@MainActor
private let mir4DRadialToolParameterCoordinatorBootstrap =
    RadialToolParameterCoordinator.shared
