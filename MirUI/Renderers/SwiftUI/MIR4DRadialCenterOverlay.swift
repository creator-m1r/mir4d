import SwiftUI

/// Full-screen presentation layer for the creative radial menu.
/// The menu is centered on the display and temporarily creates a calm focus field around the scene.
struct MIR4DRadialCenterOverlay: View {
    @ObservedObject var store: RadialMenuSettingsStore
    let vector: CGVector
    let onToolActivated: (RadialMenuTool) -> Void
    let onSettings: () -> Void
    @State private var breathing = false

    private var distance: CGFloat { CGFloat(hypot(vector.dx, vector.dy)) }
    private var focus: Double { min(max(Double(distance) / 180.0, 0), 1) }

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                MIR4DSceneSoftBlur()
                    .ignoresSafeArea()
                    .allowsHitTesting(false)
                    .opacity(0.86)

                Color.black.opacity(0.10 + focus * 0.08)
                    .ignoresSafeArea()
                    .allowsHitTesting(false)

                // A quiet halo keeps the center spatially legible without becoming another control.
                Circle()
                    .stroke(MirTheme.Colors.accentBright.opacity(0.10 + focus * 0.12), lineWidth: 1)
                    .frame(width: 430 + CGFloat(focus) * 36, height: 430 + CGFloat(focus) * 36)
                    .scaleEffect(breathing ? 1.015 : 0.985)
                    .blur(radius: 0.2)
                    .allowsHitTesting(false)

                RadialMenuView(
                    store: store,
                    center: CGPoint(x: proxy.size.width / 2, y: proxy.size.height / 2),
                    vector: vector,
                    onToolActivated: onToolActivated,
                    onSettings: onSettings
                )
            }
            .animation(.easeOut(duration: 0.18), value: distance)
        }
        .transition(.opacity.combined(with: .scale(scale: 0.96)))
        .onAppear {
            withAnimation(.easeInOut(duration: 1.8).repeatForever(autoreverses: true)) {
                breathing = true
            }
        }
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
