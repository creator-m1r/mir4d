import Foundation
import CoreGraphics

@MainActor
final class SketchSelectionController: ObservableObject {
    @Published private(set) var selectedIDs: Set<UUID> = []
    @Published private(set) var hoveredID: UUID?
    @Published private(set) var dragStart: CGPoint?
    @Published private(set) var dragCurrent: CGPoint?

    func hover(_ id: UUID?) {
        hoveredID = id
    }

    func select(_ id: UUID, additive: Bool = false) {
        if additive {
            if selectedIDs.contains(id) { selectedIDs.remove(id) }
            else { selectedIDs.insert(id) }
        } else {
            selectedIDs = [id]
        }
    }

    func clear() {
        selectedIDs.removeAll()
        hoveredID = nil
        cancelDrag()
    }

    func beginDrag(at point: CGPoint) {
        guard !selectedIDs.isEmpty else { return }
        dragStart = point
        dragCurrent = point
    }

    func updateDrag(to point: CGPoint) {
        guard dragStart != nil else { return }
        dragCurrent = point
    }

    func cancelDrag() {
        dragStart = nil
        dragCurrent = nil
    }

    func endDrag() -> CGVector? {
        guard let start = dragStart, let current = dragCurrent else {
            cancelDrag()
            return nil
        }
        let delta = CGVector(dx: current.x - start.x, dy: current.y - start.y)
        cancelDrag()
        return delta
    }
}
