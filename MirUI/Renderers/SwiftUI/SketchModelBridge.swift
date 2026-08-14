import Foundation
import CoreGraphics
import Combine

/// UI-side command bridge for the sketch engine.
/// The actual C++ bridge can later replace the closures without changing the UI.
@MainActor
final class SketchModelBridge: ObservableObject {
    @Published private(set) var geometryRevision: UInt64 = 0
    @Published private(set) var constraintRevision: UInt64 = 0

    private var lineCreator: ((CGPoint, CGPoint) -> Void)?
    private var constraintCreator: ((String, UInt32, UInt32) -> Void)?

    func connect(
        lineCreator: @escaping (CGPoint, CGPoint) -> Void,
        constraintCreator: @escaping (String, UInt32, UInt32) -> Void
    ) {
        self.lineCreator = lineCreator
        self.constraintCreator = constraintCreator
    }

    func createLine(from start: CGPoint, to end: CGPoint) {
        lineCreator?(start, end)
        geometryRevision &+= 1
    }

    func addCoincidentConstraint(
        type: String = "Coincident",
        geometry: UInt32,
        target: UInt32
    ) {
        constraintCreator?(type, geometry, target)
        constraintRevision &+= 1
    }
}
