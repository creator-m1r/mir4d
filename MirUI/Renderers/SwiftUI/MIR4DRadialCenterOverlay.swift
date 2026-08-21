import SwiftUI

/// Full-screen presentation layer for the creative radial menu.
/// The menu is centered on the display and temporarily creates a calm focus field around the scene.
struct MIR4DRadialCenterOverlay: View {
    @ObservedObject var store: RadialMenuSettingsStore
    let vector: CGVector
    let appState: CADAppState
    let onToolActivated: (RadialMenuTool) -> Void
    let onSettings: () -> Void

    @State private var breathing = false
    @State private var keyPanel: Int? = nil
    @State private var keyTool: Int? = nil

    private var distance: CGFloat { CGFloat(hypot(vector.dx, vector.dy)) }
    private var focus: Double { min(max(Double(distance) / 180.0, 0), 1) }

    private func visiblePanels() -> [RadialMenuPanel] {
        let ctx = RadialMenuContext(appState: appState)
        return RadialMenuView.visiblePanels(store: store, context: ctx)
    }
    private func visibleTools(panel: RadialMenuPanel) -> [RadialMenuTool] {
        let ctx = RadialMenuContext(appState: appState)
        return RadialMenuView.visibleTools(panel: panel, store: store, context: ctx)
    }

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
                    appState: appState,
                    onToolActivated: onToolActivated,
                    onSettings: onSettings,
                    forcedPanelIndex: keyPanel,
                    forcedToolIndex: keyTool
                )
            }
            .animation(.easeOut(duration: 0.18), value: distance)
            .focusable()
            .onChange(of: vector) { _, _ in keyPanel = nil; keyTool = nil }
            .onKeyPress(.leftArrow) { self.handleKeyPress(.leftArrow) }
            .onKeyPress(.rightArrow) { self.handleKeyPress(.rightArrow) }
            .onKeyPress(.upArrow) { self.handleKeyPress(.upArrow) }
            .onKeyPress(.downArrow) { self.handleKeyPress(.downArrow) }
            .onKeyPress(.tab) { self.handleKeyPress(.tab) }
            .onKeyPress(.return) { self.handleKeyPress(.return) }
            .onKeyPress(.space) { self.handleKeyPress(.space) }
            .onKeyPress(.escape) { self.handleKeyPress(.escape) }
        }
        .transition(.opacity.combined(with: .scale(scale: 0.96)))
        .onAppear {
            withAnimation(.easeInOut(duration: 1.8).repeatForever(autoreverses: true)) {
                breathing = true
            }
        }
    }

    private func handleKeyPress(_ key: KeyEquivalent) -> KeyPress.Result {
        let panels = visiblePanels()
        guard !panels.isEmpty else { return .ignored }
        if key == .escape {
            if keyTool != nil { keyTool = nil }
            else if keyPanel != nil { keyPanel = nil }
            return .handled
        }
        if key == .tab {
            if keyPanel == nil { keyPanel = 0 }
            if keyTool == nil { keyTool = 0 }
            return .handled
        }
        if key == .return || key == .space {
            let pi = keyPanel ?? 0
            guard panels.indices.contains(pi) else { return .ignored }
            let tools = visibleTools(panel: panels[pi])
            let tool = (keyTool != nil && tools.indices.contains(keyTool!)) ? tools[keyTool!] : tools.first
            if let tool = tool { onToolActivated(tool) }
            return .handled
        }
        let fwd = (key == .rightArrow || key == .downArrow)
        if keyPanel == nil { keyPanel = 0 }
        guard let pi = keyPanel, panels.indices.contains(pi) else { return .ignored }
        let tools = visibleTools(panel: panels[pi])
        if keyTool != nil {
            guard !tools.isEmpty else { keyTool = nil; return .handled }
            var i = keyTool ?? 0
            i = fwd ? (i + 1) % tools.count : (i - 1 + tools.count) % tools.count
            keyTool = i
        } else {
            var i = keyPanel ?? 0
            i = fwd ? (i + 1) % panels.count : (i - 1 + panels.count) % panels.count
            keyPanel = i
        }
        return .handled
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
