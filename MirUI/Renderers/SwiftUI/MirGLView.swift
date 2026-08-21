import SwiftUI
import AppKit
import QuartzCore
import os

// MARK: - Notifications

extension Notification.Name {
    static let mir4DImportMesh = Notification.Name("MIR4D.ImportMesh")
    static let mir4DImportStepBRep = Notification.Name("MIR4D.ImportStepBRep")
    static let mir4DExportStepBRep = Notification.Name("MIR4D.ExportStepBRep")
    static let mir4DExportStl = Notification.Name("MIR4D.ExportSTL")
    static let mir4DExportStep = Notification.Name("MIR4D.ExportSTEP")
    static let mir4DCreateBox = Notification.Name("MIR4D.CreateBox")
    static let mir4DFitViewport = Notification.Name("MIR4D.FitViewport")
    static let mir4DCameraProjectionRequested = Notification.Name("MIR4D.CameraProjectionRequested")
    static let mir4DCreateWorkPlane = Notification.Name("MIR4D.CreateWorkPlane")
    static let mir4DOpenProject = Notification.Name("MIR4D.OpenProject")
    static let mir4DStartWorkspace = Notification.Name("MIR4D.StartWorkspace")
    static let mir4DSketchSolved = Notification.Name("MIR4D.SketchSolved")
    static let mir4DVFXTrigger = Notification.Name("MIR4D.VFXTrigger")
    static let mir4DSelectionModeChanged = Notification.Name("MIR4D.SelectionModeChanged")
}

// MARK: - OpenGL / MirEngine View

final class MirGLCustomView: NSView {

    // MARK: Callbacks

    var onSelectionChanged: ((UInt64, Int32, UInt64) -> Void)?
    var onIOError: ((String) -> Void)?
    var onCameraOrientationChanged:
        ((Double, Double, Double) -> Void)?

    // MARK: Work plane gating (ТЗ: цветные плоскости — только при создании эскиза)

    var appState: CADAppState?
    private var lastSyncedWorkbench: CADWorkbench? = nil

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
    // `OSAllocatedUnfairLock` (not `NSLock`) provides priority inheritance, so a
    // User-interactive DisplayLink thread never inverts priority behind a
    // Default-QoS engine accessor (Xcode Hang Risk 17459).
    nonisolated private static let engineLock = OSAllocatedUnfairLock(initialState: ())

    // Renders one frame. NSView.displayLink(target:selector:) fires on the
    // main run loop, so this is MainActor-isolated; the lock still protects
    // against any non-main MirEngine access.
    @objc
    private func renderTick(_ displayLink: CADisplayLink) {
        // Собственная VFX-подсистема MIR4D: продвигаем симуляцию частиц
        // на dt кадра (рисование частиц происходит внутри MirEngineRender
        // через зарегистрированный OpenGL- sink).
        let dt = Float(min(max(displayLink.duration, 0.001), 0.1))
        MIRVFX.update(dt)

        // ТЗ §3/§7: callback не должен трогать MainActor-состояние вне lock.
        // Считываем указатель engine под lock, затем рендерим под тем же lock.
        MirGLCustomView.engineLock.lock()
        let vp = viewport
        if let vp {
            MirEngineRender(vp)
        }
        MirGLCustomView.engineLock.unlock()
    }

    // Renders the very first frame synchronously right after engine setup, so
    // the startup scene is visible immediately even before the display link
    // fires (also lets the MIR4D_SCREENSHOT=1 FBO capture run headlessly).
    private func renderFirstFrameIfNeeded() {

        guard viewport != nil else {
            return
        }

        MirGLCustomView.engineLock.lock()
        MirEngineRender(viewport)
        MirGLCustomView.engineLock.unlock()
    }

    private var importObserver: NSObjectProtocol?
    private var importStepBRepObserver: NSObjectProtocol?
    private var exportStepBRepObserver: NSObjectProtocol?
    private var exportObserver: NSObjectProtocol?
    private var exportStepObserver: NSObjectProtocol?
    private var createBoxObserver: NSObjectProtocol?
    private var fitViewportObserver: NSObjectProtocol?
    private var cameraPresetObserver: NSObjectProtocol?
    private var cameraProjectionObserver: NSObjectProtocol?
    private var cameraOrbitObserver: NSObjectProtocol?
    private var cameraAnimationTimer: Timer?
    private var cameraTransitionStart: (theta: Double, phi: Double, distance: Double)?
    private var cameraTransitionTarget: (theta: Double, phi: Double, distance: Double)?
    private var cameraTransitionStartTime: CFTimeInterval = 0
    private var cameraTransitionDeltaTheta: Double = 0
    private let cameraTransitionDuration: CFTimeInterval = 0.38
    private var workPlaneObserver: NSObjectProtocol?
    private var sketchObserver: NSObjectProtocol?
    private var vfxObserver: NSObjectProtocol?
    private var vfxDemoTimer: Timer?
    private var selectionModeObserver: NSObjectProtocol?
    private var workbenchObserverToken: UUID?

    // MARK: Mouse interaction

    private var leftMouseDownPoint: NSPoint?
    private var leftMouseDidDrag = false

    // Box (rectangle) selection drag state. Triggered by holding the Option
    // key while dragging; the resulting screen rectangle is forwarded to the
    // engine as a multi (box) selection.
    private var boxSelectStart: NSPoint?
    private var boxSelectCurrent: NSPoint?
    private var isBoxSelecting = false
    private let boxOverlay = BoxSelectionOverlayView()

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
        false
    }

    override var acceptsFirstResponder: Bool {
        true
    }

    // MARK: Lifecycle

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        print("TRACE: MirGLCustomView viewDidMoveToWindow window=\(window != nil)")

        if window != nil {

            window?.makeFirstResponder(self)

            print("TRACE: observers...")
            observeImportRequests()
            observeExportRequests()
            observeExportStepRequests()
            observeCreateBoxRequests()
            observeFitViewportRequests()
            observeCameraPresetRequests()
            observeCameraProjectionRequests()
            observeCameraOrbitRequests()
            observeWorkPlaneRequests()
            observeSketchSolvedRequests()
            observeWorkbenchChanges()
            observeVFXTriggerRequests()
            observeSelectionModeChanges()
            startVFXDemoIfRequested()
            print("TRACE: observers done")

            print("TRACE: setupEngine...")
            setupEngine()
            print("TRACE: setupEngine done")
            syncWorkPlanesIfNeeded()
            startDisplayLink()
            publishCameraOrientation()
            renderFirstFrameIfNeeded()

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

        // The OpenGL context is owned process-lifetime by MIR4DModelRuntime so
        // it survives NSView remounts (launch creates the view, then AppKit
        // remounts it once). Reuse the existing context and merely rebind it to
        // the new NSView instead of destroying and recreating the (expensive)
        // NSOpenGLContext. The renderer + viewport (with their scene) are
        // recreated per mount — matching the original scene lifecycle — so a
        // project switch still resets the 3D scene correctly.
        if let existingContext = MIR4DModelRuntime.shared.glContext {

            MirGLCustomView.engineLock.lock()
            defer { MirGLCustomView.engineLock.unlock() }

            self.context = existingContext
            MirEngineSetOpenGLContextView(self.context, viewPointer)

            self.renderer = MirEngineCreateOpenGLRenderer(self.context)
            guard let renderer else {
                print("❌ MIR4D: failed to create OpenGL renderer (reuse)")
                self.context = nil
                return
            }

            guard MirEngineInitializeRenderer(renderer) else {
                print("❌ MIR4D: renderer initialization failed (reuse)")
                MirEngineDestroyRenderer(renderer)
                self.renderer = nil
                self.context = nil
                return
            }

            self.viewport = MirEngineCreateViewport(
                renderer,
                size.width,
                size.height
            )
            MIR4DModelRuntime.shared.viewport = self.viewport
            MIR4DModelRuntime.shared.renderer = self.renderer
            print("TRACE: setupEngine viewport ok (reuse): \(self.viewport != nil)")

            if let startupViewport = self.viewport {
                print("TRACE: setupEngine creating box (reuse)...")
                var startupBoxID: UInt64 = 0
                if MirEngineCreateBox(startupViewport, 2.0, 2.0, 2.0, &startupBoxID) {
                    print("MIR4D: startup cube created (objectID=\(startupBoxID))")
                }
            }

            guard self.viewport != nil else {
                print("❌ MIR4D: failed to create viewport (reuse)")
                MirEngineDestroyRenderer(renderer)
                self.renderer = nil
                self.viewport = nil
                self.context = nil
                return
            }

            print("✅ MIR4D: MirEngine reused (context rebound to new view)")
            return
        }

        print("TRACE: setupEngine acquiring lock")
        MirGLCustomView.engineLock.lock()
        defer {
            MirGLCustomView.engineLock.unlock()
        }
        print("TRACE: setupEngine lock ok")

        // IMPORTANT:
        // OpenGL context belongs to MirEngine.
        context = MirEngineCreateMacOpenGLContext(
            viewPointer,
            MirEngineSize2D(
                width: size.width,
                height: size.height
            )
        )
        print("TRACE: setupEngine context ok: \(context != nil)")

        guard let context else {

            print(
                "❌ MIR4D: failed to create OpenGL context"
            )

            return
        }

        // Context is created once and kept alive in the runtime across remounts.
        MIR4DModelRuntime.shared.glContext = context

        renderer =
            MirEngineCreateOpenGLRenderer(context)
        print("TRACE: setupEngine renderer ok: \(renderer != nil)")

        guard let renderer else {

            print(
                "❌ MIR4D: failed to create OpenGL renderer"
            )

            MirEngineDestroyOpenGLContext(context)
            MIR4DModelRuntime.shared.glContext = nil

            self.context = nil

            return
        }

        guard MirEngineInitializeRenderer(renderer) else {
            print("TRACE: setupEngine renderer init FAILED")
            print(
                "❌ MIR4D: renderer initialization failed"
            )

            MirEngineDestroyRenderer(renderer)
            MirEngineDestroyOpenGLContext(context)
            MIR4DModelRuntime.shared.glContext = nil

            self.renderer = nil
            MIR4DModelRuntime.shared.renderer = nil
            self.context = nil

            return
        }

        viewport =
            MirEngineCreateViewport(
                renderer,
                size.width,
                size.height
            )
        MIR4DModelRuntime.shared.viewport = self.viewport
        MIR4DModelRuntime.shared.renderer = self.renderer
        print("TRACE: setupEngine viewport ok: \(viewport != nil)")

        if let startupViewport = viewport {
            print("TRACE: setupEngine creating box...")
            var startupBoxID: UInt64 = 0
            if MirEngineCreateBox(startupViewport, 2.0, 2.0, 2.0, &startupBoxID) {
                print("MIR4D: startup cube created (objectID=\(startupBoxID))")
            }
            print("TRACE: setupEngine box done")
        }

        // ТЗ Этап 1: опубликовать базовые рабочие плоскости в оверлее вьюпорта.
        // Показываем их только в режиме создания эскиза; в остальных режимах
        // видна только серая горизонтальная сетка (GridPass).
        // ВАЖНО: syncWorkPlanesIfNeeded() берёт engineLock, поэтому вызывать
        // его можно только вне критической секции setupEngine (NSLock не
        // реентерабелен). Вызов перенесён в viewDidMoveToWindow после setupEngine().
        guard viewport != nil else {

            print(
                "❌ MIR4D: failed to create viewport"
            )

            MirEngineDestroyRenderer(renderer)
            MirEngineDestroyOpenGLContext(context)
            MIR4DModelRuntime.shared.glContext = nil

            self.renderer = nil
            MIR4DModelRuntime.shared.renderer = nil
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

        MIR4DLog("DISPLAYLINK", "started")
    }

    private func stopDisplayLink() {

        guard let displayLink else {

            isRunning = false

            return
        }

        isRunning = false

        displayLink.invalidate()

        self.displayLink = nil

        MIR4DLog("DISPLAYLINK", "stopped")
    }

    // MARK: Camera

    private func publishCameraOrientation() {
        // ТЗ §23: не вызывать SwiftUI-callback внутри engineLock.
        var theta: Float = 0
        var phi: Float = 0
        var distance: Float = 0

        MirGLCustomView.engineLock.lock()
        if let viewport {
            MirEngineGetCameraOrientation(
                viewport,
                &theta,
                &phi,
                &distance
            )
        }
        MirGLCustomView.engineLock.unlock()

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
            ) { notification in

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

        importStepBRepObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DImportStepBRep,
                object: nil,
                queue: .main
            ) { notification in

                guard
                    let path =
                        notification.object as? String
                else {
                    return
                }

                DispatchQueue.main.async { [weak self] in
                    self?.importStepBRep(path: path)
                }
            }

        exportStepBRepObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DExportStepBRep,
                object: nil,
                queue: .main
            ) { notification in

                guard
                    let payload =
                        notification.object as? [String: Any],
                    let path = payload["path"] as? String
                else {
                    return
                }

                let selectionOnly =
                    (payload["selectionOnly"] as? Bool) ?? false

                DispatchQueue.main.async { [weak self] in
                    self?.exportStep(
                        path: path,
                        selectionOnly: selectionOnly
                    )
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

    private func observeExportStepRequests() {

        guard exportStepObserver == nil else {
            return
        }

        exportStepObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DExportStep,
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

                    self?.exportStep(
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

                let animated =
                    (payload["animated"] as? Bool) ?? false

                DispatchQueue.main.async { [weak self] in
                    self?.applyCameraPreset(preset, animated: animated)
                }
            }
    }

    private func applyCameraPreset(_ preset: Int32, animated: Bool) {

        NSLog("MIR4D camera preset requested: %d", preset)

        if animated {
            var targetTheta: Float = 0
            var targetPhi: Float = 0
            var targetDistance: Float = 12
            MirEngineGetCameraPresetOrientation(
                preset,
                &targetTheta,
                &targetPhi,
                &targetDistance
            )

            MirGLCustomView.engineLock.lock()
            let hasViewport = viewport != nil
            MirGLCustomView.engineLock.unlock()

            guard hasViewport else {
                return
            }

            applyCameraOrbit(
                theta: Double(targetTheta),
                phi: Double(targetPhi),
                distance: Double(targetDistance),
                animated: true
            )
            return
        }

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

    // MARK: Camera orbit (navigation sphere)

    private func observeCameraOrbitRequests() {

        guard cameraOrbitObserver == nil else {
            return
        }

        cameraOrbitObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DCameraOrbitRequested,
                object: nil,
                queue: .main
            ) { [weak self] notification in

                guard
                    let payload = notification.userInfo,
                    let theta = payload["theta"] as? Double,
                    let phi = payload["phi"] as? Double
                else {
                    return
                }

                let distance =
                    (payload["distance"] as? Double) ?? 12.0
                let animated =
                    (payload["animated"] as? Bool) ?? true

                DispatchQueue.main.async { [weak self] in
                    self?.applyCameraOrbit(
                        theta: theta,
                        phi: phi,
                        distance: distance,
                        animated: animated
                    )
                }
            }
    }

    private func applyCameraOrbit(
        theta: Double,
        phi: Double,
        distance: Double,
        animated: Bool
    ) {
        cameraAnimationTimer?.invalidate()
        cameraAnimationTimer = nil

        MirGLCustomView.engineLock.lock()
        let activeViewport = viewport
        if let activeViewport {
            if animated {
                var startTheta: Float = 0
                var startPhi: Float = 0
                var startDistance: Float = 0
                MirEngineGetCameraOrientation(
                    activeViewport,
                    &startTheta,
                    &startPhi,
                    &startDistance
                )
                startCameraTransition(
                    from: (
                        Double(startTheta),
                        Double(startPhi),
                        Double(startDistance)
                    ),
                    to: (theta, phi, distance)
                )
            } else {
                MirEngineSetCameraOrientation(
                    activeViewport,
                    Float(theta),
                    Float(phi),
                    Float(distance)
                )
            }
        }
        MirGLCustomView.engineLock.unlock()

        // publishCameraOrientation() takes engineLock itself, so it must be
        // called outside the critical section above: NSLock is not
        // reentrant, and locking it twice from the same thread deadlocks.
        if !animated, activeViewport != nil {
            publishCameraOrientation()
        }
    }

    /// Smoothly interpolates the camera orbit to the target orientation.
    /// Runs on the main thread; the display link render thread only reads
    /// the camera state (which is written under engineLock by
    /// MirEngineSetCameraOrientation).
    private func startCameraTransition(
        from start: (theta: Double, phi: Double, distance: Double),
        to target: (theta: Double, phi: Double, distance: Double)
    ) {
        cameraAnimationTimer?.invalidate()

        cameraTransitionStart = start
        cameraTransitionTarget = target
        cameraTransitionStartTime = CACurrentMediaTime()
        // Unwrap the theta delta so the camera turns the short way around.
        cameraTransitionDeltaTheta = atan2(
            sin(target.theta - start.theta),
            cos(target.theta - start.theta)
        )

        let timer = Timer.scheduledTimer(
            timeInterval: 1.0 / 60.0,
            target: self,
            selector: #selector(cameraTransitionTick(_:)),
            userInfo: nil,
            repeats: true
        )
        cameraAnimationTimer = timer
        RunLoop.main.add(timer, forMode: .common)
    }

    @objc
    private func cameraTransitionTick(_ timer: Timer) {
        guard let start = cameraTransitionStart,
              let target = cameraTransitionTarget else {
            timer.invalidate()
            cameraAnimationTimer = nil
            return
        }

        let t = min(1.0, (CACurrentMediaTime() - cameraTransitionStartTime) / cameraTransitionDuration)
        let eased = t * t * (3.0 - 2.0 * t)

        let theta = start.theta + cameraTransitionDeltaTheta * eased
        let phi = start.phi + (target.phi - start.phi) * eased
        let distance = start.distance + (target.distance - start.distance) * eased

        MirGLCustomView.engineLock.lock()
        if let viewport {
            MirEngineSetCameraOrientation(
                viewport,
                Float(theta),
                Float(phi),
                Float(distance)
            )
        }
        MirGLCustomView.engineLock.unlock()

        if t >= 1.0 {
            timer.invalidate()
            cameraAnimationTimer = nil
            cameraTransitionStart = nil
            cameraTransitionTarget = nil
            publishCameraOrientation()
        }
    }

    // MARK: Work planes (ТЗ Этап 1)

    private func observeWorkPlaneRequests() {

        guard workPlaneObserver == nil else {
            return
        }

        workPlaneObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DCreateWorkPlane,
                object: nil,
                queue: .main
            ) { [weak self] notification in

                guard
                    let payload = notification.userInfo,
                    let basePlane = payload["basePlane"] as? UInt32
                else {
                    return
                }

                let offset =
                    (payload["offset"] as? Double) ?? 0.0
                let angleDeg =
                    (payload["angleDeg"] as? Double) ?? 0.0

                DispatchQueue.main.async { [weak self] in
                    self?.createWorkPlane(
                        basePlane: basePlane,
                        offset: offset,
                        angleDeg: angleDeg
                    )
                }
            }
    }

    private func createWorkPlane(
        basePlane: UInt32,
        offset: Double,
        angleDeg: Double
    ) {
        let newId = WorkPlaneController.shared.createOffsetPlane(
            basePlane: basePlane,
            offset: offset,
            angleDeg: angleDeg
        )
        guard newId != 0 else {
            return
        }

        MirGLCustomView.engineLock.lock()
        let activeRenderer = renderer
        if let activeRenderer, appState?.workbench == .sketch {
            WorkPlaneController.shared.push(to: activeRenderer)
        }
        MirGLCustomView.engineLock.unlock()

        print("MIR4D: work plane created (id=\(newId))")
    }

    // ТЗ: рабочие плоскости (цветные) показываем только в режиме создания
    // эскиза. В остальных режимах — только серая горизонтальная сетка.
    func syncWorkPlanesIfNeeded() {
        MirGLCustomView.engineLock.lock()
        defer { MirGLCustomView.engineLock.unlock() }
        guard let activeRenderer = renderer else { return }
        let wb = appState?.workbench
        if wb == lastSyncedWorkbench { return }
        lastSyncedWorkbench = wb
        if wb == .sketch {
            WorkPlaneController.shared.push(to: activeRenderer)
        } else {
            MirEnginePushWorkPlanes(activeRenderer, [])
        }
    }


    // ТЗ: при смене рабочего стола (в т.ч. вход в «Эскиз») заново
    // синхронизируем видимость рабочих плоскостей с гейтом режима.
    private func observeWorkbenchChanges() {
        guard workbenchObserverToken == nil else {
            return
        }
        workbenchObserverToken = MirEventBus.shared.subscribe { [weak self] event in
            if case .workbenchChanged = event {
                self?.syncWorkPlanesIfNeeded()
            }
        }
    }

    // MARK: Sketch overlay (ТЗ Этап 2)

    private func observeSketchSolvedRequests() {
        guard sketchObserver == nil else { return }
        sketchObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DSketchSolved,
                object: nil,
                queue: .main
            ) { [weak self] notification in
                guard
                    let corners = notification.userInfo?["corners"] as? [CGPoint],
                    corners.count >= 2
                else { return }

                var segments: [MirEngineSketchSegment] = []
                let amber: (Float, Float, Float) = (0.95, 0.85, 0.25)
                for i in 0..<corners.count {
                    let a = corners[i]
                    let b = corners[(i + 1) % corners.count]
                    segments.append(
                        MirEngineSketchSegment(
                            ax: Float(a.x), ay: Float(a.y),
                            bx: Float(b.x), by: Float(b.y),
                            color: amber
                        )
                    )
                }

                DispatchQueue.main.async { [weak self] in
                    self?.renderSketchOverlay(segments)
                }
            }
    }

    private func renderSketchOverlay(_ segments: [MirEngineSketchSegment]) {
        MirGLCustomView.engineLock.lock()
        if let activeRenderer = renderer {
            MirEnginePushSketch(activeRenderer, segments)
        }
        MirGLCustomView.engineLock.unlock()
        print("MIR4D: sketch overlay pushed (\(segments.count) segments)")
    }

    // MARK: VFX (собственная подсистема эффектов MIR4D)

    /// Запускает эффект в собственной VFX-подсистеме MIR4D.
    private func triggerVFX(_ kind: MIRVFXKind) {
        MIRVFX.trigger(kind)
    }

    private func observeVFXTriggerRequests() {
        guard vfxObserver == nil else { return }
        vfxObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DVFXTrigger,
                object: nil,
                queue: .main
            ) { [weak self] notification in
                let raw = notification.userInfo?["kind"] as? Int
                    ?? notification.object as? Int
                let kind = MIRVFXKind(rawValue: Int32(raw ?? 0)) ?? .confetti
                DispatchQueue.main.async { [weak self] in
                    self?.triggerVFX(kind)
                }
            }
    }

    /// Пробрасывает выбранный в overlay режим выделения (Body/Face/Edge/Vertex)
    /// в движок, чтобы pick-запросы фильтровались по mode.
    private func observeSelectionModeChanges() {
        guard selectionModeObserver == nil else { return }
        selectionModeObserver =
            NotificationCenter.default.addObserver(
                forName: .mir4DSelectionModeChanged,
                object: nil,
                queue: .main
            ) { [weak self] notification in
                guard let modeString = notification.userInfo?["mode"] as? String else {
                    return
                }
                // Mirrors PickKind: 0 = None, 1 = Body, 2 = Face, 3 = Edge, 4 = Vertex
                let modeIndex: Int32
                switch modeString {
                    case "body": modeIndex = 1
                    case "face": modeIndex = 2
                    case "edge": modeIndex = 3
                    case "vertex": modeIndex = 4
                    default: modeIndex = 1
                }
                DispatchQueue.main.async { [weak self] in
                    MirGLCustomView.engineLock.lock()
                    if let viewport = self?.viewport {
                        MirEngineSetSelectionFilter(viewport, modeIndex)
                    }
                    MirGLCustomView.engineLock.unlock()
                    let cursor: NSCursor = (modeString == "body") ? .arrow : .crosshair
                    cursor.set()
                }
            }
    }

    /// Опциональная демонстрация: при MIR4D_VFX_DEMO=1 периодически
    /// запускает случайные эффекты, чтобы визуально проверить работу
    /// собственного VFX-движка (выключено по умолчанию).
    private func startVFXDemoIfRequested() {
        guard ProcessInfo.processInfo.environment["MIR4D_VFX_DEMO"] != nil else {
            return
        }
        vfxDemoTimer?.invalidate()
        let timer = Timer.scheduledTimer(withTimeInterval: 2.5, repeats: true) {
            failed in
            let all: [MIRVFXKind] = [
                .confetti, .balloons, .fireworks, .rain, .hearts, .lasers,
                .sparks, .snow, .bubbles, .sparkles, .petals, .streamers,
                .scanGrid, .assemble, .smoke, .fire, .warnBurst,
                .selectPulse, .success
            ]
            let pick = all.randomElement() ?? .confetti
            MIRVFX.trigger(pick)
        }
        vfxDemoTimer = timer
        RunLoop.main.add(timer, forMode: .common)
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

        if let observer = workPlaneObserver {
            NotificationCenter.default.removeObserver(observer)
            workPlaneObserver = nil
        }

        if let observer = sketchObserver {
            NotificationCenter.default.removeObserver(observer)
            sketchObserver = nil
        }

        if let token = workbenchObserverToken {
            MirEventBus.shared.unsubscribe(token)
            workbenchObserverToken = nil
        }

        if let observer = vfxObserver {
            NotificationCenter.default.removeObserver(observer)
            vfxObserver = nil
        }
        vfxDemoTimer?.invalidate()
        vfxDemoTimer = nil

        if let observer = selectionModeObserver {
            NotificationCenter.default.removeObserver(observer)
            selectionModeObserver = nil
        }


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

        if let observer = importStepBRepObserver {
            NotificationCenter.default.removeObserver(
                observer
            )
            importStepBRepObserver = nil
        }

        if let observer = exportStepBRepObserver {
            NotificationCenter.default.removeObserver(
                observer
            )
            exportStepBRepObserver = nil
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

        if let observer = cameraOrbitObserver {
            NotificationCenter.default.removeObserver(
                observer
            )
            cameraOrbitObserver = nil
        }

        cameraAnimationTimer?.invalidate()
        cameraAnimationTimer = nil
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
            // Инвалидируем и общий указатель: MIR4DModelRuntime держит его для
            // C++-вызовов (clearHandSkeleton/setHandSkeleton). Без сброса после
            // teardown указатель остаётся висячим и следующий вызов падает с
            // EXC_BAD_ACCESS в ViewportRuntime::clearHandSkeleton().
            MIR4DModelRuntime.shared.viewport = nil
        }

        if let renderer {

            MirEngineDestroyRenderer(renderer)

            self.renderer = nil
            MIR4DModelRuntime.shared.renderer = nil
        }

        if let context {

            // The OpenGL context is owned process-lifetime by MIR4DModelRuntime:
            // detach it from this (about-to-be-discarded) NSView instead of
            // destroying it, so a subsequent NSView remount can rebind and reuse
            // it. The context itself stays alive in MIR4DModelRuntime.glContext.
            MirEngineSetOpenGLContextView(context, nil)

            self.context = nil
        }

        print(
            "🛑 MIR4D: MirEngine shutdown (context detached, kept in runtime)"
        )
        MIR4DLog("ENGINE", "shutdown (context kept=\(MIR4DModelRuntime.shared.glContext != nil) renderer=\(renderer != nil) viewport=\(viewport != nil))")
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

        return mirCString(pointer) ?? "Неизвестная ошибка MirEngine"
    }

    // MARK: Import

    private func importMesh(path: String) {

        var success = false
        var errorMessage: String?

        MirGLCustomView.engineLock.lock()
        guard let viewport else {
            MirGLCustomView.engineLock.unlock()
            onIOError?("MIR4D import: viewport is not ready")
            return
        }

        let isStep =
            path.lowercased().hasSuffix(".step") ||
            path.lowercased().hasSuffix(".stp")

        success = path.withCString { cPath in

            if isStep {
                return MirEngineImportStepBRep(
                    viewport,
                    cPath
                )
            }

            return MirEngineImportMesh(
                viewport,
                cPath
            )
        }

        if !success {
            errorMessage = engineErrorMessage(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        if success {

            print(
                "✅ MIR4D import: \(path)"
            )

        } else {

            let message = errorMessage ?? "Неизвестная ошибка MirEngine"
            onIOError?(message)

            print(
                "❌ MIR4D import: \(message)"
            )
        }
    }

    // MARK: Import STEP (exact B-Rep)

    private func importStepBRep(path: String) {

        var success = false
        var errorMessage: String?

        MirGLCustomView.engineLock.lock()
        guard let viewport else {
            MirGLCustomView.engineLock.unlock()
            onIOError?("MIR4D STEP B-Rep import: viewport is not ready")
            return
        }

        success = path.withCString { cPath in
            MirEngineImportStepBRep(
                viewport,
                cPath
            )
        }

        if !success {
            errorMessage = engineErrorMessage(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        if success {

            print(
                "✅ MIR4D STEP B-Rep import: \(path)"
            )

        } else {

            let message = errorMessage ?? "Неизвестная ошибка MirEngine"
            onIOError?(message)

            print(
                "❌ MIR4D STEP B-Rep import: \(message)"
            )
        }
    }

    // MARK: Export STEP (exact B-Rep)

    private func exportStepBRep(
        path: String,
        selectionOnly: Bool
    ) {

        var success = false
        var errorMessage: String?

        MirGLCustomView.engineLock.lock()
        guard let viewport else {
            MirGLCustomView.engineLock.unlock()
            onIOError?("MIR4D STEP B-Rep export: viewport is not ready")
            return
        }

        success = path.withCString { cPath in
            MirEngineExportStepBRep(
                viewport,
                cPath,
                selectionOnly
            )
        }

        if !success {
            errorMessage = engineErrorMessage(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        if success {

            print(
                "✅ MIR4D STEP B-Rep export: \(path)"
            )

        } else {

            let message = errorMessage ?? "Неизвестная ошибка MirEngine"
            onIOError?(message)

            print(
                "❌ MIR4D STEP B-Rep export: \(message)"
            )
        }
    }

    // MARK: Export STL

    private func exportStl(
        path: String,
        selectionOnly: Bool
    ) {

        var success = false
        var errorMessage: String?

        MirGLCustomView.engineLock.lock()
        guard let viewport else {
            MirGLCustomView.engineLock.unlock()
            onIOError?("MIR4D export: viewport is not ready")
            return
        }

        success = path.withCString { cPath in

            MirEngineExportStl(
                viewport,
                cPath,
                selectionOnly
            )
        }

        if !success {
            errorMessage = engineErrorMessage(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        if success {

            print(
                "✅ MIR4D export STL: \(path)"
            )

        } else {

            let message = errorMessage ?? "Неизвестная ошибка MirEngine"
            onIOError?(message)

            print(
                "❌ MIR4D export STL: \(message)"
            )
        }
    }

    // MARK: Export STEP

    private func exportStep(
        path: String,
        selectionOnly: Bool
    ) {

        var success = false
        var errorMessage: String?

        MirGLCustomView.engineLock.lock()
        guard let viewport else {
            MirGLCustomView.engineLock.unlock()
            onIOError?("MIR4D STEP export: viewport is not ready")
            return
        }

        success = path.withCString { cPath in

            MirEngineExportStep(
                viewport,
                cPath,
                selectionOnly
            )
        }

        if !success {
            errorMessage = engineErrorMessage(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        if success {

            print(
                "✅ MIR4D export STEP: \(path)"
            )

        } else {

            let message = errorMessage ?? "Неизвестная ошибка MirEngine"
            onIOError?(message)

            print(
                "❌ MIR4D export STEP: \(message)"
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

        var objectID: UInt64 = 0
        var success = false
        var errorMessage: String?

        MirGLCustomView.engineLock.lock()
        guard let viewport else {
            MirGLCustomView.engineLock.unlock()
            onIOError?("MIR4D create body: viewport is not ready")
            return
        }

        success = MirEngineCreateBox(
            viewport,
            width,
            depth,
            height,
            &objectID
        )

        if !success {
            errorMessage = engineErrorMessage(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        // ТЗ §22/§24: UI- и SwiftUI-callbacks вне engineLock.
        if success {

            if let bodyID {
                Task { @MainActor in
                    MIR4DModelRuntime.shared.registerViewportEngineID(
                        bodyID: bodyID,
                        engineObjectID: objectID
                    )
                }
            }

            publishSelection()

            print(
                "✅ MIR4D create body: \(objectID)"
            )

        } else {

            let message = errorMessage ?? "Неизвестная ошибка MirEngine"
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

        var objectID: UInt64 = 0
        var kind: Int32 = 0
        var elementId: UInt64 = 0

        MirGLCustomView.engineLock.lock()
        if let viewport {
            objectID = MirEngineGetSelectedObjectId(viewport)
            kind = MirEngineGetSelectionKind(viewport)
            elementId = MirEngineGetSelectionElementId(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        onSelectionChanged?(objectID, kind, elementId)

        if let viewport {
            let faceArea = MirEngineGetSelectionFaceArea(viewport)
            let edgeLength = MirEngineGetSelectionEdgeLength(viewport)
            appState?.setSelectionElementMetrics(faceArea: faceArea, edgeLength: edgeLength)
            appState?.setSelectionCount(Int(MirEngineGetSelectionCount(viewport)))
        }
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

    // MARK: Deinit

    deinit {
        // ТЗ §37: диагностика жизненного цикла. Полная очистка (DisplayLink,
        // observers, engine) выполняется в viewDidMoveToWindow(window == nil)
        // на MainActor — deinit не должен обращаться к MainActor-свойствам.
        MIR4DLog("LIFECYCLE", "MirGLCustomView deinit")
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

        // Option-drag starts a rectangle (box) selection instead of orbit.
        if event.modifierFlags.contains(.option) {
            isBoxSelecting = true
            boxSelectStart = localPoint
            boxSelectCurrent = localPoint
            updateBoxOverlay()
            return
        }

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

        // Finish a rectangle selection drag.
        if isBoxSelecting {
            defer {
                isBoxSelecting = false
                boxSelectStart = nil
                boxSelectCurrent = nil
                boxOverlay.isHidden = true
            }

            guard
                let start = boxSelectStart,
                let end = boxSelectCurrent
            else {
                return
            }

            let a = enginePoint(start)
            let b = enginePoint(end)
            let additive = event.modifierFlags.contains(.shift)

            MirGLCustomView.engineLock.lock()
            if let viewport {
                MirEngineViewportBoxSelect(
                    viewport,
                    a.0, a.1, b.0, b.1,
                    additive
                )
            }
            MirGLCustomView.engineLock.unlock()

            publishSelection()
            return
        }

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

        var objectID: UInt64 = 0

        MirGLCustomView.engineLock.lock()
        if let viewport {
            let point = enginePoint(localPoint)
            let addToSelection = event.modifierFlags.contains(.shift)

            MirEngineViewportClick(
                viewport,
                point.0,
                point.1,
                addToSelection
            )

            objectID = MirEngineGetSelectedObjectId(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        publishSelection()
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

        // Rectangle selection drag: update the overlay and the engine rect.
        if isBoxSelecting {
            boxSelectCurrent =
                convert(
                    event.locationInWindow,
                    from: nil
                )
            updateBoxOverlay()
            return
        }

        radialActivationWorkItem?.cancel()
        radialActivationWorkItem = nil

        leftMouseDidDrag = true

        forwardMouseMove(event)
        publishCameraOrientation()
    }

    // MARK: Box selection overlay

    private func updateBoxOverlay() {
        if boxOverlay.superview == nil {
            boxOverlay.translatesAutoresizingMaskIntoConstraints = false
            addSubview(boxOverlay)
            NSLayoutConstraint.activate([
                boxOverlay.leadingAnchor.constraint(equalTo: leadingAnchor),
                boxOverlay.trailingAnchor.constraint(equalTo: trailingAnchor),
                boxOverlay.topAnchor.constraint(equalTo: topAnchor),
                boxOverlay.bottomAnchor.constraint(equalTo: bottomAnchor)
            ])
        }

        guard
            let start = boxSelectStart,
            let end = boxSelectCurrent
        else {
            boxOverlay.isHidden = true
            return
        }

        let x = min(start.x, end.x)
        let y = min(start.y, end.y)
        let w = abs(end.x - start.x)
        let h = abs(end.y - start.y)
        boxOverlay.rect = NSRect(x: x, y: y, width: w, height: h)
        boxOverlay.isHidden = (w < 1 && h < 1)
        boxOverlay.needsDisplay = true
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

        // Передаём курсор в renderer для подсветки плоскости под указателем.
        let scale = backingScale()
        let w = bounds.width * scale
        let h = bounds.height * scale
        if w > 0, h > 0, let activeRenderer = renderer {
            let ndcX = point.0 / Float(w) * 2.0 - 1.0
            let ndcY = point.1 / Float(h) * 2.0 - 1.0
            MirEngineSetCursor(activeRenderer, ndcX, ndcY, true)
        }
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
        if let activeRenderer = renderer {
            MirEngineSetCursor(activeRenderer, 0, 0, false)
        }
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

        var objectID: UInt64 = 0

        MirGLCustomView.engineLock.lock()
        if let viewport {
            MirEngineViewportDragCancel(viewport)
            objectID = MirEngineGetSelectedObjectId(viewport)
            if objectID != 0 {
                MirEngineClearSelection(viewport)
            }
        }
        MirGLCustomView.engineLock.unlock()

        if objectID != 0 {

            publishSelection()
        }
    }

    /// Delete / Backspace: remove the primary selection through MirEngine.
    private func deleteSelectedObject() {

        var objectID: UInt64 = 0
        var removed = false

        MirGLCustomView.engineLock.lock()
        if let viewport {
            objectID = MirEngineGetSelectedObjectId(viewport)
            removed = MirEngineDeleteSelectedObject(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        guard removed else {
            return
        }

        // Keep the persisted model (navigation tree) in sync with the scene.
        if objectID > 0 {
            MIR4DModelRuntime.shared.removeBody(
                forViewportObjectID: objectID
            )
        }

        publishSelection()
    }

    /// Cmd+Z (undo) / Cmd+Shift+Z (redo) of scene commands.
    private func undoOrRedo(isShift: Bool) {

        var applied = false

        MirGLCustomView.engineLock.lock()
        if let viewport {
            applied = isShift
                ? MirEngineRedo(viewport)
                : MirEngineUndo(viewport)
        }
        MirGLCustomView.engineLock.unlock()

        guard applied else {
            return
        }

        // The scene may have changed identity; drop the stale selection so
        // the UI (properties panel) reflects the reverted state.
        publishSelection()
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
        ((UInt64, Int32, UInt64) -> Void)?

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

/// Transparent overlay used to draw the rectangle (box) selection marquee.
/// It never receives mouse events (hitTest returns nil) so drags keep flowing
/// to the underlying viewport.
private final class BoxSelectionOverlayView: NSView {

    var rect: NSRect = .zero

    override var isOpaque: Bool { false }

    override func draw(_ dirtyRect: NSRect) {
        guard !rect.isEmpty else { return }
        NSColor.controlAccentColor.withAlphaComponent(0.18).setFill()
        NSColor.controlAccentColor.setStroke()
        let path = NSBezierPath(rect: rect)
        path.lineWidth = 1.0 / max(1.0, window?.backingScaleFactor ?? 1.0)
        path.fill()
        path.stroke()
    }

    override func hitTest(_ point: NSPoint) -> NSView? { nil }
}
