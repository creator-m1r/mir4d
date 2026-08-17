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
        GeometryReader { proxy in
            let metrics = metrics(for: proxy.size)
            ZStack {
                content
                    .contentShape(Rectangle())
                    .blur(radius: radialVisible ? metrics.blurRadius : 0)
                    .animation(.easeOut(duration: 0.20), value: radialVisible)
                    .gesture(sceneGesture(metrics: metrics))

                if radialVisible {
                    Color.clear
                        .contentShape(Rectangle())
                        .gesture(radialGesture(metrics: metrics))
                }
            }
        }
        .onDisappear { coordinator.cancel() }
    }

    private func metrics(for size: CGSize) -> MIR4DTouchInteractionPolicy.Metrics {
        #if os(iOS)
        return MIR4DTouchInteractionPolicy.metrics(for: .iPad, shortestSide: min(size.width, size.height))
        #else
        // SwiftUI cannot reliably distinguish mouse and trackpad at this layer.
        // The platform input adapter can later provide the precise pointer kind.
        return MIR4DTouchInteractionPolicy.metrics(for: .macTrackpad, shortestSide: min(size.width, size.height))
        #endif
    }

    private func sceneGesture(metrics: MIR4DTouchInteractionPolicy.Metrics) -> some Gesture {
        SimultaneousGesture(
            DragGesture(minimumDistance: metrics.sceneGestureMinimumDistance, coordinateSpace: .local)
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

    private func radialGesture(metrics: MIR4DTouchInteractionPolicy.Metrics) -> some Gesture {
        DragGesture(minimumDistance: 0, coordinateSpace: .local)
            .onChanged { value in
                let vector = MIR4DTouchGeometry.clamped(
                    MIR4DTouchGeometry.radialVector(from: gestureOrigin, to: value.location),
                    maximum: metrics.radialMaximum
                )
                coordinator.radialActivationDistance = metrics.radialDeadZone
                coordinator.radialSubmenuDistance = metrics.radialSecondOrbit
                coordinator.maximumRadialDistance = metrics.radialMaximum
                coordinator.updateRadial(vector: vector)
            }
            .onEnded { value in
                let vector = MIR4DTouchGeometry.clamped(
                    MIR4DTouchGeometry.radialVector(from: gestureOrigin, to: value.location),
                    maximum: metrics.radialMaximum
                )
                if !MIR4DTouchGeometry.isInsideRadialDeadZone(vector, radius: metrics.radialDeadZone) {
                    onRadialCommit(vector)
                }
                radialVisible = false
                coordinator.endGesture()
                gestureOrigin = .zero
            }
    }

    /// Requests the radial interface. The gesture origin is the touch start,
    /// while the rendered menu itself remains centred by the host viewport.
    func activateRadial(at location: CGPoint) {
        gestureOrigin = location
        radialVisible = true
        coordinator.beginTouches(count: 2)
        coordinator.updateRadial(vector: .zero)
    }
}
