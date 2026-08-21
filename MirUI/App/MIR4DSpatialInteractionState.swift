import SwiftUI

/// Canonical interaction state shared by touch, pointer and future hand input.
struct MIR4DSpatialInteractionState: Equatable {
    var isActive = false
    var isFocused = false
    var position: CGPoint = .zero
    var translation: CGSize = .zero
    var scale: CGFloat = 1
    var rotation: Angle = .zero
    var confidence: Double = 1
    var source: Source = .direct

    enum Source: String, Equatable {
        case direct
        case touch
        case pointer
        case hand
    }

    static let idle = Self()
}

@MainActor
final class MIR4DSpatialInteractionStore: ObservableObject {
    @Published private(set) var state = MIR4DSpatialInteractionState.idle

    func begin(source: MIR4DSpatialInteractionState.Source, position: CGPoint = .zero) {
        state.isActive = true
        state.source = source
        state.position = position
        state.confidence = 1
    }

    func update(
        position: CGPoint? = nil,
        translation: CGSize? = nil,
        scale: CGFloat? = nil,
        rotation: Angle? = nil,
        confidence: Double? = nil
    ) {
        if let position { state.position = position }
        if let translation { state.translation = translation }
        if let scale { state.scale = min(max(scale, 0.5), 2.0) }
        if let rotation { state.rotation = rotation }
        if let confidence { state.confidence = min(max(confidence, 0), 1) }
    }

    func end() {
        state.isActive = false
        state.translation = .zero
        state.scale = 1
        state.rotation = .zero
    }

    func focus() {
        state.isFocused = true
    }

    func unfocus() {
        state.isFocused = false
    }

    func reset() {
        withAnimation(.spring(response: 0.28, dampingFraction: 0.86)) {
            state = .idle
        }
    }
}

extension View {
    func mir4DSpatialInteraction(
        store: MIR4DSpatialInteractionStore,
        source: MIR4DSpatialInteractionState.Source = .direct
    ) -> some View {
        simultaneousGesture(
            DragGesture(minimumDistance: 4)
                .onChanged { value in
                    store.begin(source: source, position: value.location)
                    store.update(position: value.location, translation: value.translation)
                }
                .onEnded { _ in
                    store.end()
                }
        )
        .onTapGesture {
            store.focus()
        }
    }
}
