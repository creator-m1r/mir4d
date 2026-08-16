import SwiftUI
import AppKit
import QuartzCore

// MARK: - Notifications

extension Notification.Name {
    static let mir4DImportMesh = Notification.Name("MIR4D.ImportMesh")
    static let mir4DExportStl = Notification.Name("MIR4D.ExportSTL")
    static let mir4DCreateBox = Notification.Name("MIR4D.CreateBox")
    static let mir4DFitViewport = Notification.Name("MIR4D.FitViewport")
    static let mir4DCameraProjectionRequested = Notification.Name("MIR4D.CameraProjectionRequested")
}

// MARK: - OpenGL / MirEngine View

final class MirGLCustomView: NSView {

    // MARK: Callbacks

    var onSelectionChanged: ((UInt64) -> Void)?
    var onIOError: ((String) -> Void)?
    var onCameraOrientationChanged:
        ((Double, Double, Double) -> Void)?

    // MARK: MirEngine objects

    private var context: UnsafeMutableRawPointer?
    private var renderer: UnsafeMutableRawPointer?
    private var viewport: UnsafeMutableRawPointer?

    // MARK: Rendering

    private var displayLink: CADisplayLink?
    private var isRunning = false

    // Protects MirEngine objects from display-link / UI races.
    // Static so the display-link callback can take it without touching any
    // MainActor-isolated instance state (NSView subclasses are @MainActor).
    //
    // nonisolated(unsafe): MirGLCustomView inherits from @MainActor NSView, so
    // its static members are main-isolated by default in Swift 6. NSLock is
    // thread-safe by design, so the marker is sound.
    nonisolated private static let engineLock = NSLock()

    // Renders one frame. NSView.displayLink(target:selector:) fires on the
    // main run loop, so this is MainActor-isolated; the lock still protects
    // against any non-main MirEngine access.
    @objc
    private func renderTick(_ displayLink: CADisplayLink) {

        guard viewport != nil else {
            return
        }

        MirGLCustomView.engineLock.lock()
        MirEngineRender(viewport)
        MirGLCustomView.engineLock.unlock()
    }

    // MARK: Notifications

    private var importObserver: NSObjectProtocol?
    private var exportObserver: NSObjectProtocol?
    private var createBoxObserver: NSObjectProtocol?
    private var fitViewportObserver: NSObjectProtocol?
    private var cameraPresetObserver: NSObjectProtocol?
    private var cameraProjectionObserver: NSObjectProtocol?

    // MARK: Mouse interaction

    private var leftMouseDownPoint: NSPoint?
    private var leftMouseDidDrag = false

    // MARK: Radial menu

    private enum RadialTrigger {
        case keyboard
        case middleMouse
        case leftMouseHold
    }

    private var radialMouseDownPoint: NSPoint?
    private var radialMenuActive = false
    private var radialKeyboardActive = false
    private var radialActivationWorkItem: DispatchWorkItem?

    private let radialSettings =
        RadialMenuSettingsStore.shared

    // MARK: NSView

    override var isOpaque: Bool {
        true
    }

    override var wantsUpdateLayer: Bool {
        true
    }

    override var acceptsFirstResponder: Bool {
        true
    }

    // MARK: Lifecycle

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()

        if window != nil {

            window?.makeFirstResponder(self)

            observeImportRequests()
            observeExportRequests()
            observeCreateBoxRequests()
            observeFitViewportRequests()
            observeCameraPresetRequests()
            observeCameraProjectionRequests()

            setupEngine()
            startDisplayLink()
            publishCameraOrientation()

        } else {

            stopDisplayLink()
            removeObservers()
            endRadialMenu(commit: false)
            shutdownEngine()
        }
    }

    override func layout() {
        super.layout()
        resizeEngine()
    }

    override func viewDidChangeBackingProperties() {
        super.viewDidChangeBackingProperties()
        resizeEngine()
    }

    // MARK: Geometry

    private func backingScale() -> CGFloat {
        window?.backingScaleFactor
            ?? NSScreen.main?.backingScaleFactor
            ?? 1.0
    }

    private func physicalSize()
        -> (width: UInt32, height: UInt32)
    {
        let scale = backingScale()

        let width = max(
            Int((bounds.width * scale).rounded(.up)),
            1
        )

        let height = max(
            Int((bounds.height * scale).rounded(.up)),
            1
        )

        return (
            UInt32(width),
            UInt32(height)
        )
    }

    private func enginePoint(
        _ point: NSPoint
    ) -> (Float, Float) {

        let scale = backingScale()

        return (
            Float(point.x * scale),
            Float(point.y * scale)
        )
    }

    // MARK: Engine initialization

    private func setupEngine() {

        guard context == nil else {
            return
        }

        guard window != nil else {
            return
        }

        let size = physicalSize()

        let viewPointer =
            Unmanaged.passUnretained(self).toOpaque()

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        // IMPORTANT:
        // OpenGL context belongs to MirEngine.
        context = MirEngineCreateMacOpenGLContext(
            viewPointer,
            MirEngineSize2D(
                width: size.width,
                height: size.height
            )
        )

        guard let context else {

            print(
                "❌ MIR4D: failed to create OpenGL context"
            )

            return
        }

        renderer =
            MirEngineCreateOpenGLRenderer(context)

        guard let renderer else {

            print(
                "❌ MIR4D: failed to create OpenGL renderer"
            )

            MirEngineDestroyOpenGLContext(context)

            self.context = nil

            return
        }

        guard MirEngineInitializeRenderer(renderer) else {

            print(
                "❌ MIR4D: renderer initialization failed"
            )

            MirEngineDestroyRenderer(renderer)
            MirEngineDestroyOpenGLContext(context)

            self.renderer = nil
            self.context = nil

            return
        }

        viewport =
            MirEngineCreateViewport(
                renderer,
                size.width,
                size.height
            )

        if let startupViewport = viewport {
            var startupBoxID: UInt64 = 0
            if MirEngineCreateBox(startupViewport, 2.0, 2.0, 2.0, &startupBoxID) {
                print("MIR4D: startup cube created (objectID=\(startupBoxID))")
            }
        }

        guard viewport != nil else {

            print(
                "❌ MIR4D: failed to create viewport"
            )

            MirEngineDestroyRenderer(renderer)
            MirEngineDestroyOpenGLContext(context)

            self.renderer = nil
            self.context = nil

            return
        }

        print(
            "✅ MIR4D: MirEngine + OpenGL + Viewport initialized"
        )
    }

    // MARK: Rendering

    // This method is intentionally NOT actor-isolated.
    //
    // The display link fires on the main run loop but it must not
    // call any @MainActor / NSView method.
    //
    // The actual MirEngine rendering is protected by engineLock.
    //
    // IMPORTANT: the C callback must never touch Swift object state.
    // MirGLCustomView inherits NSView, which is @MainActor-isolated in the
    // modern AppKit SDK; touching `self` from the display-link thread traps
    // with EXC_BREAKPOINT (incorrect actor executor assumption).
    // The only state passed is the raw MirEngine viewport pointer, which is
    // naturally Sendable and is rendered via the pure C ABI below.

    private func resizeEngine() {

        let size = physicalSize()

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        MirEngineResize(
            viewport,
            size.width,
            size.height
        )
    }

    // MARK: Display Link

    private func startDisplayLink() {

        guard displayLink == nil else {
            return
        }

        guard viewport != nil else {
            return
        }

        let link = displayLink(
            target: self,
            selector: #selector(renderTick(_:))
        )

        link.add(to: .main, forMode: .common)

        displayLink = link
        isRunning = true

        print(
            "▶️ MIR4D: DisplayLink started"
        )
    }

    private func stopDisplayLink() {

        guard let displayLink else {

            isRunning = false

            return
        }

        isRunning = false

        displayLink.invalidate()

        self.displayLink = nil

        print(
            "⏹ MIR4D: DisplayLink stopped"
        )
    }

    // MARK: Camera

    private func publishCameraOrientation() {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        var theta: Float = 0
        var phi: Float = 0
        var distance: Float = 0

        MirEngineGetCameraOrientation(
            viewport,
            &theta,
            &phi,
            &distance
        )

        onCameraOrientationChanged?(
            Double(theta),
            Double(phi),
            Double(distance)
        )
    }

    // MARK: Notifications

    private func observeImportRequests() {

        guard importObserver == nil else {
            return
        }

        importObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DImportMesh,
                object: nil,
                queue: .main
            ) { [weak self] notification in

                guard
                    let path =
                        notification.object as? String
                else {
                    return
                }

                DispatchQueue.main.async { [weak self] in
                    self?.importMesh(path: path)
                }
            }
    }

    private func observeExportRequests() {

        guard exportObserver == nil else {
            return
        }

        exportObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DExportStl,
                object: nil,
                queue: .main
            ) { [weak self] notification in

                guard
                    let payload =
                        notification.object as? [String: Any],
                    let path =
                        payload["path"] as? String
                else {
                    return
                }

                let selectionOnly =
                    (payload["selectionOnly"] as? Bool)
                    ?? false

                DispatchQueue.main.async { [weak self] in

                    self?.exportStl(
                        path: path,
                        selectionOnly: selectionOnly
                    )
                }
            }
    }

    private func observeCreateBoxRequests() {

        guard createBoxObserver == nil else {
            return
        }

        createBoxObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DCreateBox,
                object: nil,
                queue: .main
            ) { [weak self] notification in

                guard
                    let payload =
                        notification.object as? [String: Any],
                    let width =
                        payload["width"] as? Double,
                    let depth =
                        payload["depth"] as? Double,
                    let height =
                        payload["height"] as? Double
                else {
                    return
                }

                let bodyID =
                    (payload["bodyID"] as? String)
                    .flatMap(UUID.init(uuidString:))

                DispatchQueue.main.async { [weak self] in

                    self?.createBox(
                        width: width,
                        depth: depth,
                        height: height,
                        bodyID: bodyID
                    )
                }
            }
    }

    private func observeFitViewportRequests() {

        guard fitViewportObserver == nil else {
            return
        }

        fitViewportObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DFitViewport,
                object: nil,
                queue: .main
            ) { [weak self] _ in

                DispatchQueue.main.async { [weak self] in
                    self?.fitViewport()
                }
            }
    }

    private func observeCameraPresetRequests() {

        guard cameraPresetObserver == nil else {
            return
        }

        cameraPresetObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DCameraPresetRequested,
                object: nil,
                queue: .main
            ) { [weak self] notification in

                guard
                    let payload =
                        notification.userInfo,
                    let raw =
                        payload["preset"] as? String,
                    let preset =
                        Int32(raw)
                else {
                    return
                }

                DispatchQueue.main.async { [weak self] in
                    self?.applyCameraPreset(preset)
                }
            }
    }

    private func applyCameraPreset(_ preset: Int32) {

        NSLog("MIR4D camera preset requested: %d", preset)

        MirGLCustomView.engineLock.lock()
        let activeViewport = viewport
        if let activeViewport {
            MirEngineSetActiveCameraPreset(
                activeViewport,
                preset
            )
        }
        MirGLCustomView.engineLock.unlock()

        // publishCameraOrientation() takes engineLock itself, so it must be
        // called outside the critical section above: NSLock is not
        // reentrant, and locking it twice from the same thread deadlocks.
        if activeViewport != nil {
            publishCameraOrientation()
        }
    }

    private func observeCameraProjectionRequests() {

        guard cameraProjectionObserver == nil else {
            return
        }

        cameraProjectionObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DCameraProjectionRequested,
                object: nil,
                queue: .main
            ) { [weak self] notification in

                guard
                    let payload =
                        notification.userInfo,
                    let raw =
                        payload["projection"] as? Int,
                    raw == 0 || raw == 1
                else {
                    return
                }

                DispatchQueue.main.async { [weak self] in
                    self?.applyCameraProjection(Int32(raw))
                }
            }
    }

    private func applyCameraProjection(_ projection: Int32) {

        MirGLCustomView.engineLock.lock()
        let activeViewport = viewport
        if let activeViewport {
            MirEngineSetCameraProjection(
                activeViewport,
                projection
            )
        }
        MirGLCustomView.engineLock.unlock()
    }

    private func removeObservers() {

        if let observer = importObserver {
            NotificationCenter.default.removeObserver(
                observer
            )
            importObserver = nil
        }

        if let observer = exportObserver {
            NotificationCenter.default.removeObserver(
                observer
            )
            exportObserver = nil
        }

        if let observer = createBoxObserver {
            NotificationCenter.default.removeObserver(
                observer
            )
            createBoxObserver = nil
        }

        if let observer = fitViewportObserver {
            NotificationCenter.default.removeObserver(
                observer
            )
            fitViewportObserver = nil
        }

        if let observer = cameraPresetObserver {
            NotificationCenter.default.removeObserver(
                observer
            )
            cameraPresetObserver = nil
        }

        if let observer = cameraProjectionObserver {
            NotificationCenter.default.removeObserver(
                observer
            )
            cameraProjectionObserver = nil
        }
    }

    // MARK: Engine shutdown

    private func shutdownEngine() {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        if let viewport {

            MirEngineDestroyViewport(viewport)

            self.viewport = nil
        }

        if let renderer {

            MirEngineDestroyRenderer(renderer)

            self.renderer = nil
        }

        if let context {

            MirEngineDestroyOpenGLContext(context)

            self.context = nil
        }

        print(
            "🛑 MIR4D: MirEngine shutdown"
        )
    }

    // MARK: Errors

    private func engineErrorMessage(
        _ viewport: UnsafeMutableRawPointer
    ) -> String {

        guard
            let pointer =
                MirEngineGetLastError(viewport)
        else {
            return "Неизвестная ошибка MirEngine"
        }

        return String(cString: pointer)
    }

    // MARK: Import

    private func importMesh(path: String) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {

            onIOError?(
                "MIR4D import: viewport is not ready"
            )

            return
        }

        let success =
            path.withCString { cPath in

                MirEngineImportMesh(
                    viewport,
                    cPath
                )
            }

        if success {

            print(
                "✅ MIR4D import: \(path)"
            )

        } else {

            let message =
                engineErrorMessage(viewport)

            onIOError?(message)

            print(
                "❌ MIR4D import: \(message)"
            )
        }
    }

    // MARK: Export STL

    private func exportStl(
        path: String,
        selectionOnly: Bool
    ) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {

            onIOError?(
                "MIR4D export: viewport is not ready"
            )

            return
        }

        let success =
            path.withCString { cPath in

                MirEngineExportStl(
                    viewport,
                    cPath,
                    selectionOnly
                )
            }

        if success {

            print(
                "✅ MIR4D export STL: \(path)"
            )

        } else {

            let message =
                engineErrorMessage(viewport)

            onIOError?(message)

            print(
                "❌ MIR4D export STL: \(message)"
            )
        }
    }

    // MARK: Create Box

    private func createBox(
        width: Double,
        depth: Double,
        height: Double,
        bodyID: UUID? = nil
    ) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {

            onIOError?(
                "MIR4D create body: viewport is not ready"
            )

            return
        }

        var objectID: UInt64 = 0

        let success =
            MirEngineCreateBox(
                viewport,
                width,
                depth,
                height,
                &objectID
            )

        if success {

            if let bodyID {
                Task { @MainActor in
                    MIR4DModelRuntime.shared.registerViewportEngineID(
                        bodyID: bodyID,
                        engineObjectID: objectID
                    )
                }
            }

            onSelectionChanged?(objectID)

            print(
                "✅ MIR4D create body: \(objectID)"
            )

        } else {

            let message =
                engineErrorMessage(viewport)

            onIOError?(message)

            print(
                "❌ MIR4D create body: \(message)"
            )
        }
    }

    // MARK: Fit All

    private func fitViewport() {

        MirGLCustomView.engineLock.lock()
        let viewportAvailable = viewport != nil
        if let viewport {
            MirEngineFitViewport(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        // publishCameraOrientation() takes engineLock itself, so it must be
        // called outside the critical section above: NSLock is not
        // reentrant, and locking it twice from the same thread deadlocks
        // the main thread (and starves the display link render tick).
        if viewportAvailable {
            publishCameraOrientation()
        }
    }

    // MARK: Selection

    private func publishSelection() {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        let objectID =
            MirEngineGetSelectedObjectId(viewport)

        onSelectionChanged?(objectID)
    }

    // MARK: Radial Menu

    private func scheduleRadialMenu(
        at point: NSPoint,
        trigger: RadialTrigger
    ) {

        radialActivationWorkItem?.cancel()

        guard radialSettings.settings.enabled else {
            return
        }

        switch trigger {
        case .keyboard:
            if !radialSettings.settings.keyboardTriggerEnabled {
                return
            }
        case .middleMouse:
            if !radialSettings.settings.middleMouseTriggerEnabled {
                return
            }
        case .leftMouseHold:
            if !radialSettings.settings.leftMouseHoldTriggerEnabled {
                return
            }
        }

        let delay =
            max(
                radialSettings.settings.holdDuration,
                0
            )

        if delay == 0 {

            beginRadialMenu(
                at: point,
                trigger: trigger
            )

            return
        }

        let workItem =
            DispatchWorkItem { [weak self] in

                self?.beginRadialMenu(
                    at: point,
                    trigger: trigger
                )
            }

        radialActivationWorkItem = workItem

        DispatchQueue.main.asyncAfter(
            deadline: .now() + delay,
            execute: workItem
        )
    }

    private func beginRadialMenu(
        at point: NSPoint,
        trigger: RadialTrigger
    ) {

        guard !radialMenuActive else {
            return
        }

        radialActivationWorkItem = nil
        radialMouseDownPoint = point
        radialMenuActive = true
        radialKeyboardActive =
            trigger == .keyboard

        let x =
            bounds.width > 0
            ? point.x / bounds.width
            : 0.5

        let y =
            bounds.height > 0
            ? 1.0 - point.y / bounds.height
            : 0.5

        NotificationCenter.default.post(
            name: .mir4DRadialMenuBegan,
            object: nil,
            userInfo: [
                "x": x,
                "y": y,
                "dx": 0.0,
                "dy": 0.0
            ]
        )
    }

    private func updateRadialMenu(
        at point: NSPoint
    ) {

        guard
            radialMenuActive,
            let start = radialMouseDownPoint
        else {
            return
        }

        let dx =
            point.x - start.x

        let dy =
            start.y - point.y

        NotificationCenter.default.post(
            name: .mir4DRadialMenuMoved,
            object: nil,
            userInfo: [
                "dx": dx,
                "dy": dy
            ]
        )
    }

    private func endRadialMenu(
        commit: Bool,
        at point: NSPoint? = nil
    ) {

        radialActivationWorkItem?.cancel()
        radialActivationWorkItem = nil

        guard radialMenuActive else {
            return
        }

        let start =
            radialMouseDownPoint
            ?? point
            ?? .zero

        let current =
            point
            ?? start

        let dx =
            current.x - start.x

        let dy =
            start.y - current.y

        NotificationCenter.default.post(
            name: .mir4DRadialMenuEnded,
            object: nil,
            userInfo: [
                "commit": commit,
                "dx": dx,
                "dy": dy
            ]
        )

        radialMouseDownPoint = nil
        radialMenuActive = false
        radialKeyboardActive = false
    }

    private func currentMousePoint() -> NSPoint? {

        guard let window else {
            return nil
        }

        let screen =
            NSEvent.mouseLocation

        return convert(
            window.convertPoint(
                fromScreen: screen
            ),
            from: nil
        )
    }

    // MARK: Keyboard

    override func keyDown(
        with event: NSEvent
    ) {

        let isRadialKey =
            event.charactersIgnoringModifiers == "`"
            || event.charactersIgnoringModifiers == "~"

        if isRadialKey {

            scheduleRadialMenu(
                at:
                    currentMousePoint()
                    ?? NSPoint(
                        x: bounds.midX,
                        y: bounds.midY
                    ),
                trigger: .keyboard
            )

            return
        }

        if event.keyCode == 53 {

            if radialMenuActive {

                endRadialMenu(
                    commit: false
                )

                return
            }

            cancelDragOrClearSelection()

            return
        }

        if event.keyCode == 51 ||
            event.keyCode == 117 {

            deleteSelectedObject()

            return
        }

        if event.modifierFlags.contains(.command),
           event.charactersIgnoringModifiers?.lowercased() == "z" {

            let isShift =
                event.modifierFlags.contains(.shift)

            undoOrRedo(isShift: isShift)

            return
        }

        if event.keyCode == 3 ||
            event.charactersIgnoringModifiers == "f" ||
            event.charactersIgnoringModifiers == "F" {

            fitViewport()

            return
        }

        super.keyDown(with: event)
    }

    override func keyUp(
        with event: NSEvent
    ) {

        let isRadialKey =
            event.charactersIgnoringModifiers == "`"
            || event.charactersIgnoringModifiers == "~"

        if isRadialKey {

            endRadialMenu(
                commit: true,
                at: currentMousePoint()
            )

            return
        }

        super.keyUp(with: event)
    }

    // MARK: Mouse

    override func mouseDown(
        with event: NSEvent
    ) {

        window?.makeFirstResponder(self)

        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        leftMouseDownPoint = localPoint

        leftMouseDidDrag = false

        forwardMouseDown(event)

        scheduleRadialMenu(
            at: localPoint,
            trigger: .leftMouseHold
        )
    }

    override func mouseUp(
        with event: NSEvent
    ) {

        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        if radialMenuActive {

            endRadialMenu(
                commit: true,
                at: localPoint
            )

            leftMouseDownPoint = nil
            leftMouseDidDrag = false

            return
        }

        if let start = leftMouseDownPoint {

            let dx =
                localPoint.x - start.x

            let dy =
                localPoint.y - start.y

            leftMouseDidDrag =
                leftMouseDidDrag
                || (dx * dx + dy * dy) > 9.0
        }

        forwardMouseUp(event)

        let wasClick =
            !leftMouseDidDrag

        leftMouseDownPoint = nil
        leftMouseDidDrag = false

        guard
            wasClick,
            event.buttonNumber == 0
        else {
            return
        }

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        let point =
            enginePoint(localPoint)

        let addToSelection =
            event.modifierFlags.contains(.shift)

        MirEngineViewportClick(
            viewport,
            point.0,
            point.1,
            addToSelection
        )

        let objectID =
            MirEngineGetSelectedObjectId(
                viewport
            )

        onSelectionChanged?(objectID)
    }

    override func mouseDragged(
        with event: NSEvent
    ) {

        if radialMenuActive {

            updateRadialMenu(
                at:
                    convert(
                        event.locationInWindow,
                        from: nil
                    )
            )

            return
        }

        radialActivationWorkItem?.cancel()
        radialActivationWorkItem = nil

        leftMouseDidDrag = true

        forwardMouseMove(event)
        publishCameraOrientation()
    }

    // MARK: Hover

    override func updateTrackingAreas() {

        super.updateTrackingAreas()

        trackingAreas.forEach {
            removeTrackingArea($0)
        }

        let trackingArea =
            NSTrackingArea(
                rect: .zero,
                options: [
                    .activeInKeyWindow,
                    .mouseMoved,
                    .mouseEnteredAndExited,
                    .inVisibleRect
                ],
                owner: self,
                userInfo: nil
            )

        addTrackingArea(trackingArea)
    }

    override func mouseMoved(
        with event: NSEvent
    ) {

        if radialMenuActive {
            return
        }

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        let point =
            enginePoint(localPoint)

        MirEngineViewportHover(
            viewport,
            point.0,
            point.1
        )
    }

    override func mouseExited(
        with event: NSEvent
    ) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        MirEngineViewportHoverClear(viewport)
    }

    override func rightMouseDown(
        with event: NSEvent
    ) {

        forwardMouseDown(event)
    }

    override func rightMouseUp(
        with event: NSEvent
    ) {

        forwardMouseUp(event)
    }

    override func rightMouseDragged(
        with event: NSEvent
    ) {

        forwardMouseMove(event)
        publishCameraOrientation()
    }

    override func otherMouseDown(
        with event: NSEvent
    ) {

        if event.buttonNumber == 2 {

            scheduleRadialMenu(
                at:
                    convert(
                        event.locationInWindow,
                        from: nil
                    ),
                trigger: .middleMouse
            )

            return
        }

        forwardMouseDown(event)
    }

    override func otherMouseUp(
        with event: NSEvent
    ) {

        if event.buttonNumber == 2 {

            endRadialMenu(
                commit: true,
                at:
                    convert(
                        event.locationInWindow,
                        from: nil
                    )
            )

            return
        }

        forwardMouseUp(event)
    }

    override func otherMouseDragged(
        with event: NSEvent
    ) {

        if event.buttonNumber == 2 {

            if radialMenuActive {

                updateRadialMenu(
                    at:
                        convert(
                            event.locationInWindow,
                            from: nil
                        )
                )
            }

            return
        }

        forwardMouseMove(event)
        publishCameraOrientation()
    }

    override func scrollWheel(
        with event: NSEvent
    ) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        if event.hasPreciseScrollingDeltas {

            handleTrackpadScroll(
                event,
                viewport: viewport
            )

            return
        }

        // Mouse wheel keeps its zoom behavior, anchored at the cursor pixel
        // (industrial zoom-to-cursor).
        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        let point =
            enginePoint(localPoint)

        MirEngineViewportZoomAt(
            viewport,
            Float(event.scrollingDeltaY),
            point.0,
            point.1
        )
    }

    /// Blender-style trackpad scheme:
    /// - two fingers drag — orbit (configurable: pan / zoom)
    /// - Shift + two fingers — pan
    /// - Control + two fingers — zoom
    private func handleTrackpadScroll(
        _ event: NSEvent,
        viewport: UnsafeMutableRawPointer
    ) {

        let settings =
            MirNavigationSettingsStore.shared.settings

        let flags =
            event.modifierFlags

        let rawDX =
            Float(event.scrollingDeltaX)

        let rawDY =
            Float(event.scrollingDeltaY)

        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        let cursor =
            enginePoint(localPoint)

        if flags.contains(.control) {

            // Control + two fingers — zoom, wheel-like, anchored at the cursor.
            let zoom =
                rawDY * Float(settings.zoomSensitivity)
                * (settings.invertZoom ? -1 : 1)

            MirEngineViewportZoomAt(
                viewport,
                zoom,
                cursor.0,
                cursor.1
            )

            return
        }

        if flags.contains(.shift) {

            // Shift + two fingers — pan.
            MirEngineViewportPan(
                viewport,
                rawDX,
                -rawDY
            )

            return
        }

        switch settings.trackpadGesture {

        case .orbit:

            let dx =
                rawDX * Float(settings.orbitSensitivity)
                * (settings.invertOrbitX ? -1 : 1)

            let dy =
                -rawDY * Float(settings.orbitSensitivity)
                * (settings.invertOrbitY ? -1 : 1)

            MirEngineViewportOrbit(
                viewport,
                dx,
                dy
            )

        case .pan:

            MirEngineViewportPan(
                viewport,
                rawDX,
                -rawDY
            )

        case .zoom:

            let zoom =
                rawDY * Float(settings.zoomSensitivity)
                * (settings.invertZoom ? -1 : 1)

            MirEngineViewportZoomAt(
                viewport,
                zoom,
                cursor.0,
                cursor.1
            )
        }
    }

    override func magnify(
        with event: NSEvent
    ) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        // Trackpad pinch: spreading the fingers zooms in,
        // pinching them together zooms out. Anchored at the cursor pixel.
        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        let point =
            enginePoint(localPoint)

        MirEngineViewportZoomAt(
            viewport,
            Float(event.magnification),
            point.0,
            point.1
        )
    }

    // MARK: Engine mouse forwarding

    /// Esc handling: abort an active drag first, otherwise clear the selection.
    private func cancelDragOrClearSelection() {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        MirEngineViewportDragCancel(viewport)

        let objectID =
            MirEngineGetSelectedObjectId(viewport)

        if objectID != 0 {

            MirEngineClearSelection(viewport)

            onSelectionChanged?(0)
        }
    }

    /// Delete / Backspace: remove the primary selection through MirEngine.
    private func deleteSelectedObject() {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        let objectID =
            MirEngineGetSelectedObjectId(viewport)

        let removed =
            MirEngineDeleteSelectedObject(viewport)

        guard removed else {
            return
        }

        // Keep the persisted model (navigation tree) in sync with the scene.
        if objectID > 0 {
            MIR4DModelRuntime.shared.removeBody(
                forViewportObjectID: objectID
            )
        }

        onSelectionChanged?(0)
    }

    /// Cmd+Z (undo) / Cmd+Shift+Z (redo) of scene commands.
    private func undoOrRedo(isShift: Bool) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        let applied =
            isShift
            ? MirEngineRedo(viewport)
            : MirEngineUndo(viewport)

        guard applied else {
            return
        }

        // The scene may have changed identity; drop the stale selection so
        // the UI (properties panel) reflects the reverted state.
        onSelectionChanged?(0)
    }

    private func forwardMouseDown(
        _ event: NSEvent
    ) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        let point =
            enginePoint(localPoint)

        MirEngineViewportMouseDown(
            viewport,
            Int32(event.buttonNumber),
            point.0,
            point.1
        )
    }

    private func forwardMouseUp(
        _ event: NSEvent
    ) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        let point =
            enginePoint(localPoint)

        MirEngineViewportMouseUp(
            viewport,
            Int32(event.buttonNumber),
            point.0,
            point.1
        )
    }

    private func forwardMouseMove(
        _ event: NSEvent
    ) {

        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        let point =
            enginePoint(localPoint)

        MirEngineViewportMouseMove(
            viewport,
            point.0,
            point.1
        )
    }
}

// MARK: - SwiftUI bridge

struct MirGLView: NSViewRepresentable {

    var onSelectionChanged:
        ((UInt64) -> Void)?

    var onIOError:
        ((String) -> Void)?

    var onCameraOrientationChanged:
        ((Double, Double, Double) -> Void)?

    func makeNSView(
        context: Context
    ) -> MirGLCustomView {

        let view =
            MirGLCustomView()

        view.onSelectionChanged =
            onSelectionChanged

        view.onIOError =
            onIOError

        view.onCameraOrientationChanged =
            onCameraOrientationChanged

        return view
    }

    func updateNSView(
        _ nsView: MirGLCustomView,
        context: Context
    ) {

        nsView.onSelectionChanged =
            onSelectionChanged

        nsView.onIOError =
            onIOError

        nsView.onCameraOrientationChanged =
            onCameraOrientationChanged
    }
}
