import SwiftUI
import AppKit

/// Focus transition behind the spatial menu.
///
/// The scene flows through:
/// ```text
/// normal → focus transition → spatial menu
/// ```
/// using a soft dual blur, dimming and slightly reduced contrast. Closing the menu
/// reverses the transition. Every step is animated — no abrupt appearing.
struct MIRSpatialMenuBlur: View {
    /// 0 = scene untouched, 1 = fully focused menu state.
    var intensity: Double

    var body: some View {
        ZStack {
            MIRSpatialMenuBackdropBlur()
                .opacity(0.35 + intensity * 0.55)

            Color.black
                .opacity(0.06 + intensity * 0.12)

            // Slightly reduce contrast of the dimmed scene (dual effect).
            Color.black
                .opacity(0.05 + intensity * 0.06)
                .blendMode(.softLight)
        }
        .animation(MIRSpatialMenuAnimation.blurTransition, value: intensity)
        .allowsHitTesting(false)
        .ignoresSafeArea()
    }
}

/// Native macOS backdrop blur. It lives above the scene and below the fan,
/// so the model becomes softly defocused while the command space stays crisp.
private struct MIRSpatialMenuBackdropBlur: NSViewRepresentable {
    func makeNSView(context: Context) -> NSVisualEffectView {
        let view = NSVisualEffectView()
        view.material = .hudWindow
        view.blendingMode = .withinWindow
        view.state = .active
        view.alphaValue = 1
        return view
    }

    func updateNSView(_ nsView: NSVisualEffectView, context: Context) {
        nsView.material = .hudWindow
        nsView.blendingMode = .withinWindow
        nsView.state = .active
    }
}