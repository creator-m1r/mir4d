import SwiftUI

/// Platform-neutral interaction surface for MIR 4D.
/// It translates touch/pointer gestures into high-level interaction state;
/// CAD commands remain outside this layer.
struct MIR4DInteractionSurface<Content: View>: View {
    @StateObject private var coordinator: MIR4DTouchInteractionCoordinator
    @State private var gestureOrigin: CGPoint = .zero
    @State private var radialVisible = false
    @State private var lastMagnification: CGFloat = 1
    @State private var lastRotation: Angle = .zero

    let content: Content
    let onRadialCommit: (CGVector) -> Void

    init(
        onRadialCommit: @escaping (CGVector) -> Void = { _ in },
        @ViewBuilder content: () -> Content
    ) {
        _coordinator = StateObject(wrappedValue: MIR4DTouchInteractionCoordinator())
        self.onRadialCommit = onRadialCommit
        self.content = content()
    }

    var body: some View {
        ZStack {
            content
                .contentShape(Rectangle())
                .gesture(sceneGesture)

            if radialVisible {
                Color.clear
                    .contentShape(Rectangle())
                    .gesture(radialGesture)
            }
        }
        .onDisappear { coordinator.cancel() }
    }

    private var sceneGesture: some Gesture {
        SimultaneousGesture(
            DragGesture(minimumDistance: 4, coordinateSpace: .local)
                .onChanged { value in
                    if !radialVisible {
                        if gestureOrigin == .zero { gestureOrigin = value.startLocation }
                        coordinator.updateOneFinger(
                            translation: value.translation,
                            longPressActive: radialVisible
                        )
                    }
                }
                .onEnded { _ in
                    if !radialVisible { coordinator.endGesture() }
                    gestureOrigin = .zero
                },
            MagnificationGesture()
                .onChanged { value in
                    guard !radialVisible else { return }
                    let delta = value / lastMagnification
                    lastMagnification = value
                    coordinator.updateTwoFinger(
                        translation: coordinator.translation,
                        scale: delta,
                        rotation: coordinator.rotation
                    )
                }
                .onEnded { _ in
                    lastMagnification = 1
                    if !radialVisible { coordinator.endGesture() }
                }
        )
    }

    private var radialGesture: some Gesture {
        DragGesture(minimumDistance: 0, coordinateSpace: .local)
            .onChanged { value in
                let vector = MIR4DTouchGeometry.clamped(
                    MIR4DTouchGeometry.radialVector(from: gestureOrigin, to: value.location),
                    maximum: coordinator.maximumRadialDistance
                )
                coordinator.updateRadial(vector: vector)
            }
            .onEnded { value in
                let vector = MIR4DTouchGeometry.clamped(
                    MIR4DTouchGeometry.radialVector(from: gestureOrigin, to: value.location),
                    maximum: coordinator.maximumRadialDistance
                )
                if !MIR4DTouchGeometry.isInsideRadialDeadZone(vector, radius: coordinator.radialActivationDistance) {
                    onRadialCommit(vector)
                }
                radialVisible = false
                coordinator.endGesture()
                gestureOrigin = .zero
            }
    }

    /// Call from the platform input layer when a two-finger hold or `]` hold
    /// requests the radial interface. The menu remains centered on the display.
    func activateRadial(at location: CGPoint) {
        gestureOrigin = location
        radialVisible = true
        coordinator.beginTouches(count: 2)
        coordinator.updateRadial(vector: .zero)
    }
}
