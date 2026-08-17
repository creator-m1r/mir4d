import SwiftUI

/// Unified viewport composition for MIR 4D.
/// Keeps viewport-specific overlays out of CADMainView and makes the workspace
/// extensible without introducing a second camera or selection state.
struct MIR4DViewportWorkspace: View {
    @ObservedObject var appState: CADAppState
    let registry: CADCommandRegistry
    var onCommandPalette: (() -> Void)? = nil

    var body: some View {
        ZStack(alignment: .bottom) {
            ViewportRepresentable(
                appState: appState,
                onModelDrop: { _ in }
            )
            .frame(maxWidth: .infinity, maxHeight: .infinity)

            NavigationSphereView(appState: appState)
                .padding(.trailing, 14)
                .padding(.top, 14)
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
                .allowsHitTesting(true)

            if appState.selection.hasSelection {
                ContextualToolbarView(
                    appState: appState,
                    registry: registry,
                    onCommandPalette: onCommandPalette
                )
                .padding(.horizontal, 18)
                .padding(.bottom, 14)
                .transition(.move(edge: .bottom).combined(with: .opacity))
            }
        }
        .clipped()
        .animation(.easeOut(duration: 0.16), value: appState.selection.hasSelection)
    }
}
