import SwiftUI

/// Unified viewport composition for MIR 4D.
/// Camera and selection remain owned by CADAppState / MirGLCustomView.
struct MIR4DViewportWorkspace: View {
    @ObservedObject var appState: CADAppState
    let registry: CADCommandRegistry
    @Binding var cameraTheta: Double
    @Binding var cameraPhi: Double
    @Binding var cameraDistance: Double
    @Binding var isOrthographic: Bool
    var onIOError: ((String) -> Void)? = nil
    var onCommandPalette: (() -> Void)? = nil

    var body: some View {
        ZStack(alignment: .bottom) {
            ViewportRepresentable(
                appState: appState,
                onSelectionChanged: { objectID, kind, elementId in
                    let mapped: CADSelectionKind = {
                        switch kind {
                            case 1: return .vertex
                            case 2: return .edge
                            case 3: return .face
                            case 4: return .body
                            default: return .none
                        }
                    }()
                    let finalKind: CADSelectionKind = objectID > 0 ? mapped : .none
                    appState.setSelection(
                        ids: objectID > 0 ? ["\(objectID)"] : [],
                        kind: finalKind,
                        elementId: objectID > 0 ? elementId : 0
                    )
                },
                onIOError: { message in onIOError?(message) },
                onCameraOrientationChanged: { theta, phi, distance in
                    cameraTheta = theta
                    cameraPhi = phi
                    cameraDistance = distance
                }
            )
            .frame(maxWidth: .infinity, maxHeight: .infinity)

            NavigationSphereView(theta: cameraTheta, phi: cameraPhi, distance: cameraDistance, isOrthographic: isOrthographic)
                .opacity(0.65)
                .scaleEffect(0.56)
                .fixedSize()
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
                .padding(.top, 14)
                .padding(.trailing, 14)

            // Debug / assist: отдельный режим визуализации скелета кистей.
            MIRHandSkeletonModeControl()
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
                .padding(.top, 14)
                .padding(.leading, 14)

            if appState.selection.hasSelection {
                ContextualToolbarView(appState: appState, registry: registry)
                    .padding(.horizontal, 18)
                    .padding(.bottom, 14)
                    .transition(.move(edge: .bottom).combined(with: .opacity))
            }
        }
        .clipped()
        .animation(.easeOut(duration: 0.16), value: appState.selection.hasSelection)
    }
}
