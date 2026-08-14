import Foundation
import CoreGraphics

struct SketchLineBridgeState: Equatable {
    let geometryID: UInt32
    let startX: Double
    let startY: Double
    let endX: Double
    let endY: Double
    let length: Double
    let angleDegrees: Double
    let horizontal: Bool
    let vertical: Bool
}

struct SketchSessionBridge {
    typealias SelectedLineProvider = (_ geometryID: UInt32) -> SketchLineBridgeState?
    typealias SetParameterHandler = (_ geometryID: UInt32, _ parameter: SketchLineParameterUI, _ value: Double) -> Bool
    typealias SelectionHandler = (_ geometryID: UInt32?, _ additive: Bool) -> Void
    typealias HistoryHandler = () -> Bool

    let selectedLineProvider: SelectedLineProvider?
    let setParameterHandler: SetParameterHandler?
    let selectionHandler: SelectionHandler?
    let undoHandler: HistoryHandler?
    let redoHandler: HistoryHandler?

    init(
        selectedLine: SelectedLineProvider? = nil,
        setParameter: SetParameterHandler? = nil,
        selection: SelectionHandler? = nil,
        undo: HistoryHandler? = nil,
        redo: HistoryHandler? = nil
    ) {
        self.selectedLineProvider = selectedLine
        self.setParameterHandler = setParameter
        self.selectionHandler = selection
        self.undoHandler = undo
        self.redoHandler = redo
    }

    func selectedLine(id: UInt32) -> SketchLineParametersUI? {
        guard let state = selectedLineProvider?(id) else { return nil }
        return SketchLineParametersUI(
            geometryID: state.geometryID,
            startX: state.startX,
            startY: state.startY,
            endX: state.endX,
            endY: state.endY,
            length: state.length,
            angleDegrees: state.angleDegrees,
            horizontal: state.horizontal,
            vertical: state.vertical
        )
    }

    @discardableResult
    func setParameter(
        geometryID: UInt32,
        parameter: SketchLineParameterUI,
        value: Double
    ) -> Bool {
        setParameterHandler?(geometryID, parameter, value) ?? false
    }

    func select(geometryID: UInt32?, additive: Bool = false) {
        selectionHandler?(geometryID, additive)
    }

    @discardableResult
    func undo() -> Bool {
        undoHandler?() ?? false
    }

    @discardableResult
    func redo() -> Bool {
        redoHandler?() ?? false
    }
}
