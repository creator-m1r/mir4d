import SwiftUI

/// Two-finger manipulation layer for iPad and trackpad-ready spatial interaction.
/// It deliberately exposes semantic values so future hand tracking can reuse them.
struct MIR4DSpatialCanvasGestureModifier: ViewModifier {
    @ObservedObject var router: MIR4DProjectWindowInputRouter
    let windowID: UUID

    @GestureState private var magnification: CGFloat = 1
    @GestureState private var rotation: Angle = .zero
    @GestureState private var translation: CGSize = .zero

    func body(content: Content) -> some View {
        content
            .simultaneousGesture(combinedGesture)
            .onTapGesture {
                router.focus(windowID)
            }
    }

    private var combinedGesture: some Gesture {
        SimultaneousGesture(
            SimultaneousGesture(
                MagnifyGesture().updating($magnification) { value, state, _ in
                    state = value.magnification
                    router.send(
                        .pinch(phase: .changed, position: .zero, scale: value.magnification),
                        windowID: windowID
                    )
                },
                RotationGesture().updating($rotation) { value, state, _ in
                    state = value
                    router.send(
                        .rotate(phase: .changed, position: .zero, rotation: value),
                        windowID: windowID
                    )
                }
            ),
            DragGesture(minimumDistance: 8).updating($translation) { value, state, _ in
                state = value.translation
                router.send(
                    .move(phase: .changed, position: value.location, translation: value.translation),
                    windowID: windowID
                )
            }
        )
    }
}

extension View {
    func mir4DSpatialCanvasGesture(
        router: MIR4DProjectWindowInputRouter,
        windowID: UUID
    ) -> some View {
        modifier(MIR4DSpatialCanvasGestureModifier(router: router, windowID: windowID))
    }
}
