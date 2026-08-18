import SwiftUI

/// Semantic commands produced by touch, pointer or future hand tracking.
/// Scene and CAD code can consume intents without knowing their physical source.
enum MIR4DSpatialSceneIntent: Equatable {
    case focus(UUID)
    case pan(CGSize)
    case zoom(CGFloat)
    case rotate(Angle)
    case select(UUID)
    case moveObject(UUID, translation: CGSize)
    case sculpt(position: CGPoint, pressure: CGFloat)
    case cancel
}

@MainActor
final class MIR4DSpatialSceneIntentStore: ObservableObject {
    @Published private(set) var lastIntent: MIR4DSpatialSceneIntent?

    func send(_ intent: MIR4DSpatialSceneIntent) {
        lastIntent = intent
    }

    func clear() {
        lastIntent = nil
    }
}

/// Adapter from the window input language to scene-level semantic intent.
@MainActor
final class MIR4DSpatialSceneIntentAdapter {
    private let store: MIR4DSpatialSceneIntentStore

    init(store: MIR4DSpatialSceneIntentStore) {
        self.store = store
    }

    func handleWindowInput(_ input: MIR4DProjectWindowInput, windowID: UUID) {
        guard input.confidence >= 0.5 else { return }

        switch input.action {
        case .focus:
            store.send(.focus(windowID))
        case .move:
            store.send(.pan(input.translation))
        case .scale:
            store.send(.zoom(input.scale))
        case .rotate:
            store.send(.rotate(input.rotation))
        case .close:
            store.send(.cancel)
        }
    }
}
