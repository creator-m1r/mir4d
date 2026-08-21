import CoreGraphics

/// Connects a completed SwiftUI drag with the atomic MirEngine drag command.
/// Preview remains transient; only mouse-up reaches the engine.
@MainActor
final class SketchDragCommitBridge {
    typealias Inference = (type: String, firstGeometryID: UInt32, secondGeometryID: UInt32, confidence: Double)

    private let commit: ((UInt32, SketchDragController.Handle, CGPoint, CGPoint, [Inference]) -> Bool)?

    init(commit: ((UInt32, SketchDragController.Handle, CGPoint, CGPoint, [Inference]) -> Bool)? = nil) {
        self.commit = commit
    }

    @discardableResult
    func commitDrag(
        geometryID: UInt32,
        handle: SketchDragController.Handle,
        original: CGPoint,
        final: CGPoint,
        inferences: [Inference]
    ) -> Bool {
        commit?(geometryID, handle, original, final, inferences) ?? false
    }
}
