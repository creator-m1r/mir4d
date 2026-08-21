import SwiftUI
import AppKit

struct MIRSpatialMenuBlur: View {

    var intensity: Double

    var body: some View {
        ZStack {
            MIRSpatialMenuBackdropBlur()
                .opacity(0.35 + intensity * 0.55)

            Color.black
                .opacity(0.06 + intensity * 0.12)

            Color.black
                .opacity(0.05 + intensity * 0.06)
                .blendMode(.softLight)
        }
        .animation(MIRSpatialMenuAnimation.blurTransition, value: intensity)
        .allowsHitTesting(false)
        .ignoresSafeArea()
    }
}

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