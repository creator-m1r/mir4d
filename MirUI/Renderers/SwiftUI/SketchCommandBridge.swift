import Foundation
import CoreGraphics

@MainActor
final class SketchCommandBridge: ObservableObject {
    typealias CreateLineHandler = (_ start: CGPoint, _ end: CGPoint) -> SketchLineCommandResult
    typealias HistoryHandler = () -> Bool

    @Published private(set) var lastResult: SketchLineCommandResult?
    @Published private(set) var commandRevision: UInt64 = 0

    private let createLineHandler: CreateLineHandler?
    private let undoHandler: HistoryHandler?
    private let redoHandler: HistoryHandler?

    init(
        createLine: CreateLineHandler? = nil,
        undo: HistoryHandler? = nil,
        redo: HistoryHandler? = nil
    ) {
        self.createLineHandler = createLine
        self.undoHandler = undo
        self.redoHandler = redo
    }

    @discardableResult
    func createLine(start: CGPoint, end: CGPoint) -> SketchLineCommandResult? {
        guard let createLineHandler else { return nil }
        let result = createLineHandler(start, end)
        lastResult = result
        commandRevision &+= 1
        return result
    }

    @discardableResult
    func undo() -> Bool {
        guard let undoHandler else { return false }
        let success = undoHandler()
        if success { commandRevision &+= 1 }
        return success
    }

    @discardableResult
    func redo() -> Bool {
        guard let redoHandler else { return false }
        let success = redoHandler()
        if success { commandRevision &+= 1 }
        return success
    }
}

struct SketchLineCommandResult: Equatable {
    let success: Bool
    let geometryID: UInt32
    let constraintIDs: [UInt32]
    let inferredConstraints: [String]
    let degreesOfFreedom: Int?
    let solverStatus: String

    static let failed = SketchLineCommandResult(
        success: false,
        geometryID: 0,
        constraintIDs: [],
        inferredConstraints: [],
        degreesOfFreedom: nil,
        solverStatus: "Не выполнен"
    )
}
