import SwiftUI

/// Device-independent interaction state for future mouse, touch and hand input.
/// Gesture/hand agents can feed the same state without changing window views.
@MainActor
final class MIR4DSpatialWindowInteraction: ObservableObject {
    @Published private(set) var isDragging = false
    @Published private(set) var dragOffset: CGSize = .zero
    @Published private(set) var scale: CGFloat = 1
    @Published private(set) var rotation: Angle = .zero

    func beginDrag() {
        isDragging = true
    }

    func updateDrag(_ translation: CGSize) {
        dragOffset = translation
    }

    func endDrag() {
        isDragging = false
    }

    func updateScale(_ value: CGFloat) {
        scale = min(max(value, 0.75), 1.75)
    }

    func updateRotation(_ value: Angle) {
        rotation = value
    }

    func resetTransform() {
        withAnimation(.spring(response: 0.28, dampingFraction: 0.86)) {
            dragOffset = .zero
            scale = 1
            rotation = .zero
        }
    }
}

struct MIR4DSpatialWindowInteractionModifier: ViewModifier {
    @StateObject private var interaction = MIR4DSpatialWindowInteraction()
    let allowsTouchManipulation: Bool

    func body(content: Content) -> some View {
        content
            .offset(interaction.dragOffset)
            .scaleEffect(interaction.scale)
            .rotationEffect(interaction.rotation)
            .simultaneousGesture(allowsTouchManipulation ? dragGesture : nil)
            .simultaneousGesture(allowsTouchManipulation ? magnifyGesture : nil)
            .contextMenu {
                Button("Сбросить положение") {
                    interaction.resetTransform()
                }
            }
    }

    private var dragGesture: some Gesture {
        DragGesture(minimumDistance: 4)
            .onChanged { value in
                interaction.beginDrag()
                interaction.updateDrag(value.translation)
            }
            .onEnded { _ in
                interaction.endDrag()
            }
    }

    private var magnifyGesture: some Gesture {
        MagnifyGesture()
            .onChanged { value in
                interaction.updateScale(value.magnification)
            }
    }
}

extension View {
    /// Enables the same spatial transform layer that will later receive hand intents.
    func mir4DSpatialManipulation(enabled: Bool = true) -> some View {
        modifier(MIR4DSpatialWindowInteractionModifier(allowsTouchManipulation: enabled))
    }
}
