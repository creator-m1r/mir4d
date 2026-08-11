import SwiftUI
import AppKit

final class MirGLCustomView: NSView {

    private var context:  UnsafeMutableRawPointer?
    private var renderer: UnsafeMutableRawPointer?
    private var displayLink: CVDisplayLink?
    private var isRunning = false

    override var isOpaque: Bool { true }
    override var wantsUpdateLayer: Bool { true }

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()

        if window != nil {
            wantsBestResolutionOpenGLSurface = true
            setupEngine()
            startDisplayLink()
        } else {
            stopDisplayLink()
            shutdownEngine()
        }
    }

    override func layout() {
        super.layout()
        resizeEngine()
    }

    // MARK: - Engine lifecycle

    private func setupEngine() {
        guard context == nil else { return }

        let w = max(Int(bounds.width),  1)
        let h = max(Int(bounds.height), 1)

        let viewPtr = Unmanaged.passUnretained(self).toOpaque()

        context = MirEngineCreateMacOpenGLContext(
            viewPtr,
            MirEngineSize2D(width: UInt32(w), height: UInt32(h))
        )

        guard let context else {
            print("❌ MirEngine: failed to create OpenGL context")
            return
        }

        renderer = MirEngineCreateOpenGLRenderer(context)
        guard let renderer else {
            print("❌ MirEngine: failed to create renderer")
            return
        }

        guard MirEngineInitializeRenderer(renderer) else {
            print("❌ MirEngine: renderer initialization failed")
            return
        }

        print("✅ MirEngine + OpenGL initialized")
    }

    private func renderFrame() {
        guard let renderer, isRunning else { return }
        MirEngineRender(renderer)
    }

    private func resizeEngine() {
        guard let renderer else { return }

        let w = max(Int(bounds.width),  1)
        let h = max(Int(bounds.height), 1)

        MirEngineResize(renderer, UInt32(w), UInt32(h))
    }

    private func shutdownEngine() {
        stopDisplayLink()

        if let renderer {
            MirEngineDestroyRenderer(renderer)
            self.renderer = nil
        }
        if let context {
            MirEngineDestroyOpenGLContext(context)
            self.context = nil
        }
    }

    // MARK: - DisplayLink (60/120 fps)

    private func startDisplayLink() {
        guard displayLink == nil else { return }

        var link: CVDisplayLink?
        CVDisplayLinkCreateWithActiveCGDisplays(&link)
        guard let link else { return }

        displayLink = link
        isRunning = true

        CVDisplayLinkSetOutputCallback(link, { (_, _, _, _, _, userData) -> CVReturn in
            let view = Unmanaged<MirGLCustomView>.fromOpaque(userData!).takeUnretainedValue()
            DispatchQueue.main.async {
                view.renderFrame()
            }
            return kCVReturnSuccess
        }, Unmanaged.passUnretained(self).toOpaque())

        CVDisplayLinkStart(link)
    }

    private func stopDisplayLink() {
        isRunning = false
        if let link = displayLink {
            CVDisplayLinkStop(link)
            displayLink = nil
        }
    }

    deinit {
        shutdownEngine()
    }
}

// MARK: - SwiftUI wrapper

struct MirGLView: NSViewRepresentable {
    func makeNSView(context: Context) -> MirGLCustomView {
        MirGLCustomView()
    }

    func updateNSView(_ nsView: MirGLCustomView, context: Context) {}
}