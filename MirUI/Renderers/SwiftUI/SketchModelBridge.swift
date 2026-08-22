import Foundation
import CoreGraphics
import Combine

/// UI-side command bridge for the sketch engine.
///
/// It forwards SwiftUI intents to the authoritative `SketchSessionModel`
/// (which talks to the MirEngine `SketchSession`). It holds no geometry and
/// only bumps revision counters so views can react to committed changes.
@MainActor
final class SketchModelBridge: ObservableObject {
    @Published private(set) var geometryRevision: UInt64 = 0
    @Published private(set) var constraintRevision: UInt64 = 0

    let model: SketchSessionModel

    init(model: SketchSessionModel) {
        self.model = model
    }

    func createLine(from start: CGPoint, to end: CGPoint) {
        _ = model.createLine(from: start, to: end)
        geometryRevision &+= 1
    }

    func addCoincidentConstraint(geometry: UInt32, target: UInt32) {
        _ = model.addConstraint(type: .coincident, geometry: geometry, target: target)
        constraintRevision &+= 1
    }
}
