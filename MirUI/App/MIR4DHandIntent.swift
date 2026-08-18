import SwiftUI

/// Device-independent semantic intent emitted by a future hand-tracking agent.
/// The hand recognizer should only describe what the user is doing; it must not
/// know about CAD, rendering or SwiftUI implementation details.
struct MIR4DHandIntent: Equatable {
    enum Gesture: Equatable {
        case none
        case point
        case pinch
        case grab
        case release
        case openPalm
        case rotate
        case sculpt
    }

    enum Phase: Equatable {
        case began
        case changed
        case ended
        case cancelled
    }

    let gesture: Gesture
    let phase: Phase
    let position: CGPoint
    let secondaryPosition: CGPoint?
    let translation: CGSize
    let scale: CGFloat
    let rotation: Angle
    let pressure: CGFloat
    let confidence: Double
    let timestamp: TimeInterval

    var isReliable: Bool {
        confidence >= 0.70
    }
}

@MainActor
final class MIR4DHandIntentRouter: ObservableObject {
    @Published private(set) var lastIntent: MIR4DHandIntent?
    @Published private(set) var isTracking = false

    func receive(_ intent: MIR4DHandIntent) {
        guard intent.isReliable else { return }
        lastIntent = intent
        isTracking = intent.gesture != .none && intent.phase != .ended && intent.phase != .cancelled
    }

    func stopTracking() {
        isTracking = false
        lastIntent = nil
    }
}

/// Converts a hand gesture into the common scene intent language.
@MainActor
final class MIR4DHandSceneIntentAdapter {
    private let store: MIR4DSpatialSceneIntentStore

    init(store: MIR4DSpatialSceneIntentStore) {
        self.store = store
    }

    func handle(_ intent: MIR4DHandIntent, selectedObjectID: UUID? = nil) {
        guard intent.isReliable else { return }

        switch intent.gesture {
        case .none, .openPalm:
            return

        case .point:
            if let selectedObjectID, intent.phase == .began {
                store.send(.select(selectedObjectID))
            }

        case .pinch, .grab:
            if let selectedObjectID {
                store.send(.moveObject(selectedObjectID, translation: intent.translation))
            }

        case .release:
            store.send(.cancel)

        case .rotate:
            store.send(.rotate(intent.rotation))

        case .sculpt:
            store.send(
                .sculpt(
                    position: intent.position,
                    pressure: min(max(intent.pressure, 0), 1)
                )
            )
        }
    }
}
