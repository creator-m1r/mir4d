import Foundation

/// Boundary between SwiftUI parameter editing and MirEngine.
/// The bridge intentionally does not mutate geometry itself.
@MainActor
final class SketchGeometryEditBridge: ObservableObject {
    @Published private(set) var lastCommitted: SketchGeometryEditCommand?
    @Published private(set) var errorMessage: String?

    private let queue: SketchGeometryEditQueue

    init(queue: SketchGeometryEditQueue = SketchGeometryEditQueue()) {
        self.queue = queue
    }

    func commit(_ command: SketchGeometryEditCommand) {
        errorMessage = nil
        queue.enqueue(command)
        lastCommitted = command
    }

    func reject(_ command: SketchGeometryEditCommand, reason: String) {
        queue.remove(command)
        errorMessage = reason
    }

    func clear() {
        queue.clear()
        lastCommitted = nil
        errorMessage = nil
    }
}
