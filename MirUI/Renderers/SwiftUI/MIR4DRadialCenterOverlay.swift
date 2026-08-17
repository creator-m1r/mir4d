import SwiftUI

/// Full-screen presentation layer for the creative radial menu.
/// The menu is intentionally centered on the display, never attached to the pointer.
struct MIR4DRadialCenterOverlay: View {
    @ObservedObject var store: RadialMenuSettingsStore
    let vector: CGVector
    let onToolActivated: (RadialMenuTool) -> Void
    let onSettings: () -> Void

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                Color.black.opacity(0.08)
                    .ignoresSafeArea()
                    .allowsHitTesting(false)

                MIR4DSceneSoftBlur()
                    .ignoresSafeArea()
                    .allowsHitTesting(false)

                RadialMenuView(
                    store: store,
                    center: CGPoint(x: proxy.size.width / 2, y: proxy.size.height / 2),
                    vector: vector,
                    onToolActivated: onToolActivated,
                    onSettings: onSettings
                )
            }
        }
        .transition(.opacity)
        .animation(.easeOut(duration: 0.24), value: vector.dx)
    }
}

/// Native macOS backdrop blur. It lives above the scene and below the menu,
/// so the model becomes softly defocused while the radial command space stays crisp.
private struct MIR4DSceneSoftBlur: NSViewRepresentable {
    func makeNSView(context: Context) -> NSVisualEffectView {
        let view = NSVisualEffectView()
        view.material = .hudWindow
        view.blendingMode = .withinWindow
        view.state = .active
        view.alphaValue = 0.72
        return view
    }

    func updateNSView(_ nsView: NSVisualEffectView, context: Context) {
        nsView.material = .hudWindow
        nsView.blendingMode = .withinWindow
        nsView.state = .active
        nsView.alphaValue = 0.72
    }
}
