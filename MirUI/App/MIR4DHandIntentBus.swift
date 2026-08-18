import SwiftUI

/// Lightweight event bus between the hand-recognition agent and MIR 4D UI.
/// Producers publish semantic hand intents; consumers subscribe without coupling.
@MainActor
final class MIR4DHandIntentBus: ObservableObject {
    @Published private(set) var latest: MIR4DHandIntent?
    @Published private(set) var sequence: UInt64 = 0

    func publish(_ intent: MIR4DHandIntent) {
        guard intent.isReliable else { return }
        latest = intent
        sequence &+= 1
    }

    func reset() {
        latest = nil
    }
}

/// Converts hand intents into the common scene-intent stream.
@MainActor
final class MIR4DHandIntentBusAdapter {
    private let sceneStore: MIR4DSpatialSceneIntentStore

    init(sceneStore: MIR4DSpatialSceneIntentStore) {
        self.sceneStore = sceneStore
    }

    func consume(_ intent: MIR4DHandIntent, selectedObjectID: UUID? = nil) {
        guard intent.isReliable else { return }

        switch intent.gesture {
        case .none, .openPalm:
            return
        case .point:
            if let selectedObjectID, intent.phase == .began {
                sceneStore.send(.select(selectedObjectID))
            }
        case .pinch, .grab:
            if let selectedObjectID {
                sceneStore.send(.moveObject(selectedObjectID, translation: intent.translation))
            } else {
                sceneStore.send(.zoom(intent.scale))
            }
        case .release:
            sceneStore.send(.cancel)
        case .rotate:
            sceneStore.send(.rotate(intent.rotation))
        case .sculpt:
            sceneStore.send(.sculpt(position: intent.position, pressure: min(max(intent.pressure, 0), 1)))
        }
    }
}
