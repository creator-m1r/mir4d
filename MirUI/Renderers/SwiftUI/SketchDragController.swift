import SwiftUI

@MainActor
final class SketchDragController: ObservableObject {
    enum Handle {
        case start
        case end
    }

    private(set) var geometryID: UInt32?
    private(set) var handle: Handle?
    private(set) var originalPoint: CGPoint?
    private(set) var currentPoint: CGPoint?

    private let commitDrag: ((UInt32, Handle, CGPoint, CGPoint) -> Bool)?

    init(commitDrag: ((UInt32, Handle, CGPoint, CGPoint) -> Bool)? = nil) {
        self.commitDrag = commitDrag
    }

    func begin(
        geometryID: UInt32,
        handle: Handle,
        at point: CGPoint
    ) {
        self.geometryID = geometryID
        self.handle = handle
        self.originalPoint = point
        self.currentPoint = point
    }

    func update(to point: CGPoint) {
        guard geometryID != nil, handle != nil else { return }
        currentPoint = point
    }

    @discardableResult
    func end() -> Bool {
        guard
            let geometryID,
            let handle,
            let originalPoint,
            let currentPoint
        else {
            cancel()
            return false
        }

        let result = commitDrag?(geometryID, handle, originalPoint, currentPoint) ?? false
        cancel()
        return result
    }

    func cancel() {
        geometryID = nil
        handle = nil
        originalPoint = nil
        currentPoint = nil
    }
}
