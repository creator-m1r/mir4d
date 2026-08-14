import Foundation
import CoreGraphics

@MainActor
final class SketchLineParameterEditor: ObservableObject {
    private let setParameterHandler: ((UInt32, SketchLineParameterUI, Double) -> Bool)?

    init(setParameter: ((UInt32, SketchLineParameterUI, Double) -> Bool)? = nil) {
        self.setParameterHandler = setParameter
    }

    @discardableResult
    func set(
        geometryID: UInt32,
        parameter: SketchLineParameterUI,
        value: Double
    ) -> Bool {
        setParameterHandler?(geometryID, parameter, value) ?? false
    }
}
