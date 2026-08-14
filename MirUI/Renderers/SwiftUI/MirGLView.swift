import SwiftUI
import AppKit
import QuartzCore

// MARK: - Notifications

extension Notification.Name {
    static let mir4DImportMesh = Notification.Name("MIR4D.ImportMesh")
    static let mir4DExportStl = Notification.Name("MIR4D.ExportSTL")
    static let mir4DCreateBox = Notification.Name("MIR4D.CreateBox")
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

    private var displayLink: CVDisplayLink?
    private var isRunning = false

    // Protects MirEngine objects from display-link / UI races.
    private let engineLock = NSLock()

    // MARK: Notifications

    private var importObserver: NSObjectProtocol?
    private var exportObserver: NSObjectProtocol?
    private var createBoxObserver: NSObjectProtocol?

    // MARK: Mouse interaction

    private var leftMouseDownPoint: NSPoint?
    private var leftMouseDidDrag = false

    // MARK: Radial menu

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

        engineLock.lock()
        defer {
            engineLock.unlock()
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

    private func renderFrame() {

        guard isRunning else {
            return
        }

        engineLock.lock()
        defer {
            engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        MirEngineRender(viewport)
    }

    // This method is intentionally NOT actor-isolated.
    //
    // CVDisplayLink runs on its own realtime thread.
    // Therefore it must not call a @MainActor method.
    //
    // The actual MirEngine rendering is protected by engineLock.
    private func renderFrameFromDisplayLink() {

        renderFrame()
    }

    private func resizeEngine() {

        let size = physicalSize()

        engineLock.lock()
        defer {
            engineLock.unlock()
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

        var link: CVDisplayLink?

        let result =
            CVDisplayLinkCreateWithActiveCGDisplays(
                &link
            )

        guard result == kCVReturnSuccess,
              let link else {

            print(
                "❌ MIR4D: failed to create CVDisplayLink"
            )

            return
        }

        displayLink = link

        let userPointer =
            Unmanaged.passUnretained(self).toOpaque()

        CVDisplayLinkSetOutputCallback(
            link,
            { _, _, _, _, _, userInfo in

                guard let userInfo else {
                    return kCVReturnSuccess
                }

                let view =
                    Unmanaged<MirGLCustomView>
                        .fromOpaque(userInfo)
                        .takeUnretainedValue()

                // DO NOT call Swift @MainActor code here.
                //
                // CVDisplayLink executes on its own thread.
                // Rendering is protected by engineLock.
                view.renderFrameFromDisplayLink()

                return kCVReturnSuccess
            },
            userPointer
        )

        isRunning = true

        let startResult =
            CVDisplayLinkStart(link)

        guard startResult == kCVReturnSuccess else {

            print(
                "❌ MIR4D: failed to start CVDisplayLink"
            )

            CVDisplayLinkSetOutputCallback(
                link,
                nil,
                nil
            )

            displayLink = nil
            isRunning = false

            return
        }

        print(
            "▶️ MIR4D: CVDisplayLink started"
        )
    }

    private func stopDisplayLink() {

        guard let displayLink else {

            isRunning = false

            return
        }

        isRunning = false

        if CVDisplayLinkIsRunning(displayLink) {

            CVDisplayLinkStop(displayLink)
        }

        CVDisplayLinkSetOutputCallback(
            displayLink,
            nil,
            nil
        )

        self.displayLink = nil

        print(
            "⏹ MIR4D: CVDisplayLink stopped"
        )
    }

    // MARK: Camera

    private func publishCameraOrientation() {

        engineLock.lock()
        defer {
            engineLock.unlock()
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

                DispatchQueue.main.async { [weak self] in

                    self?.createBox(
                        width: width,
                        depth: depth,
                        height: height
                    )
                }
            }
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
    }

    // MARK: Engine shutdown

    private func shutdownEngine() {

        engineLock.lock()
        defer {
            engineLock.unlock()
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

        engineLock.lock()
        defer {
            engineLock.unlock()
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

        engineLock.lock()
        defer {
            engineLock.unlock()
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
        height: Double
    ) {

        engineLock.lock()
        defer {
            engineLock.unlock()
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

    // MARK: Selection

    private func publishSelection() {

        engineLock.lock()
        defer {
            engineLock.unlock()
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
        keyboard: Bool
    ) {

        radialActivationWorkItem?.cancel()

        guard radialSettings.settings.enabled else {
            return
        }

        if keyboard &&
            !radialSettings.settings.keyboardTriggerEnabled {
            return
        }

        if !keyboard &&
            !radialSettings.settings.middleMouseTriggerEnabled {
            return
        }

        let delay =
            max(
                radialSettings.settings.holdDuration,
                0
            )

        if delay == 0 {

            beginRadialMenu(
                at: point,
                keyboard: keyboard
            )

            return
        }

        let workItem =
            DispatchWorkItem { [weak self] in

                self?.beginRadialMenu(
                    at: point,
                    keyboard: keyboard
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
        keyboard: Bool
    ) {

        guard !radialMenuActive else {
            return
        }

        radialActivationWorkItem = nil
        radialMouseDownPoint = point
        radialMenuActive = true
        radialKeyboardActive = keyboard

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
                keyboard: true
            )

            return
        }

        if event.keyCode == 53 {

            endRadialMenu(
                commit: false
            )

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

        leftMouseDownPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

        leftMouseDidDrag = false

        forwardMouseDown(event)
    }

    override func mouseUp(
        with event: NSEvent
    ) {

        let localPoint =
            convert(
                event.locationInWindow,
                from: nil
            )

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

        engineLock.lock()
        defer {
            engineLock.unlock()
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

        leftMouseDidDrag = true

        forwardMouseMove(event)
        publishCameraOrientation()
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
                keyboard: false
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

        engineLock.lock()
        defer {
            engineLock.unlock()
        }

        guard let viewport else {
            return
        }

        MirEngineViewportScroll(
            viewport,
            Float(event.scrollingDeltaY)
        )
    }

    // MARK: Engine mouse forwarding

    private func forwardMouseDown(
        _ event: NSEvent
    ) {

        engineLock.lock()
        defer {
            engineLock.unlock()
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

        engineLock.lock()
        defer {
            engineLock.unlock()
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

        engineLock.lock()
        defer {
            engineLock.unlock()
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
