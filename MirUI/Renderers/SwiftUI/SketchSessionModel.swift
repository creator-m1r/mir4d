import Foundation
import CoreGraphics
import SwiftUI

@MainActor
final class SketchSessionModel: ObservableObject {
    @Published private(set) var solverStatus: String = "Не выполнен"
    @Published private(set) var degreesOfFreedom: Int?
    @Published private(set) var canUndo = false
    @Published private(set) var canRedo = false
    @Published private(set) var lastGeometryID: UInt32?
    @Published private(set) var inferredConstraints: [String] = []

    private let createLineHandler: ((CGPoint, CGPoint) -> SketchLineCommandResult)?
    private let undoHandler: (() -> Bool)?
    private let redoHandler: (() -> Bool)?

    init(
        createLine: ((CGPoint, CGPoint) -> SketchLineCommandResult)? = nil,
        undo: (() -> Bool)? = nil,
        redo: (() -> Bool)? = nil
    ) {
        self.createLineHandler = createLine
        self.undoHandler = undo
        self.redoHandler = redo
    }

    @discardableResult
    func createLine(from start: CGPoint, to end: CGPoint) -> SketchLineCommandResult? {
        guard let createLineHandler else { return nil }
        let result = createLineHandler(start, end)

        if result.success {
            lastGeometryID = result.geometryID
            solverStatus = result.solverStatus
            degreesOfFreedom = result.degreesOfFreedom
            inferredConstraints = result.inferredConstraints
        }

        refreshHistoryState()
        return result
    }

    @discardableResult
    func undo() -> Bool {
        let success = undoHandler?() ?? false
        if success {
            refreshHistoryState()
        }
        return success
    }

    @discardableResult
    func redo() -> Bool {
        let success = redoHandler?() ?? false
        if success {
            refreshHistoryState()
        }
        return success
    }

    func refreshHistoryState() {

        canUndo = undoHandler != nil
        canRedo = redoHandler != nil
    }
}
