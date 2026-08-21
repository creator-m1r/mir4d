import SwiftUI

struct MIR4DProjectWindowInputView<Content: View>: View {
    let windowID: UUID
    let content: () -> Content
    @ObservedObject var router: MIR4DProjectWindowInputRouter

    @State private var offset: CGSize = .zero
    @State private var scale: CGFloat = 1
    @State private var rotation: Angle = .zero
    @GestureState private var dragTranslation: CGSize = .zero
    @GestureState private var pinchScale: CGFloat = 1
    @GestureState private var rotationGesture: Angle = .zero

    init(
        windowID: UUID,
        router: MIR4DProjectWindowInputRouter,
        @ViewBuilder content: @escaping () -> Content
    ) {
        self.windowID = windowID
        self.router = router
        self.content = content
    }

    var body: some View {
        content()
            .offset(x: offset.width + dragTranslation.width, y: offset.height + dragTranslation.height)
            .scaleEffect(scale * pinchScale)
            .rotationEffect(rotation + rotationGesture)
            .simultaneousGesture(dragGesture)
            .simultaneousGesture(magnifyGesture)
            .simultaneousGesture(rotationGestureValue)
            .onTapGesture {
                router.focus(windowID)
            }
            .onChange(of: router.lastInput) { _, input in
                guard let input, router.focusedWindowID == windowID else { return }
                apply(input)
            }
    }

    private var dragGesture: some Gesture {
        DragGesture(minimumDistance: 5)
            .updating($dragTranslation) { value, state, _ in
                state = value.translation
            }
            .onChanged { value in
                router.send(
                    .move(phase: .changed, position: value.location, translation: value.translation),
                    windowID: windowID
                )
            }
            .onEnded { value in
                offset.width += value.translation.width
                offset.height += value.translation.height
                router.send(
                    .move(phase: .ended, position: value.location, translation: value.translation),
                    windowID: windowID
                )
            }
    }

    private var magnifyGesture: some Gesture {
        MagnifyGesture()
            .updating($pinchScale) { value, state, _ in
                state = value.magnification
            }
            .onChanged { value in
                router.send(
                    .pinch(phase: .changed, position: .zero, scale: value.magnification),
                    windowID: windowID
                )
            }
            .onEnded { value in
                scale = min(max(scale * value.magnification, 0.75), 1.75)
                router.send(
                    .pinch(phase: .ended, position: .zero, scale: value.magnification),
                    windowID: windowID
                )
            }
    }

    private var rotationGestureValue: some Gesture {
        RotationGesture()
            .updating($rotationGesture) { value, state, _ in
                state = value
            }
            .onChanged { value in
                router.send(
                    .rotate(phase: .changed, position: .zero, rotation: value),
                    windowID: windowID
                )
            }
            .onEnded { value in
                rotation += value
                router.send(
                    .rotate(phase: .ended, position: .zero, rotation: value),
                    windowID: windowID
                )
            }
    }

    private func apply(_ input: MIR4DProjectWindowInput) {
        switch input.action {
        case .move:
            if input.phase == .ended {
                offset.width += input.translation.width
                offset.height += input.translation.height
            }
        case .scale:
            if input.phase == .ended {
                scale = min(max(scale * input.scale, 0.75), 1.75)
            }
        case .rotate:
            if input.phase == .ended {
                rotation += input.rotation
            }
        case .focus, .close:
            break
        }
    }
}
