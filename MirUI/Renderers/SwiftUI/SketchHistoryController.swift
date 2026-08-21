import SwiftUI

@MainActor
final class SketchHistoryController: ObservableObject {
    typealias HistoryHandler = () -> Bool
    typealias AvailabilityHandler = () -> Bool

    @Published private(set) var revision: UInt64 = 0
    @Published private(set) var canUndo = false
    @Published private(set) var canRedo = false

    private let undoHandler: HistoryHandler?
    private let redoHandler: HistoryHandler?
    private let canUndoHandler: AvailabilityHandler?
    private let canRedoHandler: AvailabilityHandler?

    init(
        undo: HistoryHandler? = nil,
        redo: HistoryHandler? = nil,
        canUndo: AvailabilityHandler? = nil,
        canRedo: AvailabilityHandler? = nil
    ) {
        self.undoHandler = undo
        self.redoHandler = redo
        self.canUndoHandler = canUndo
        self.canRedoHandler = canRedo
        refresh()
    }

    @discardableResult
    func undo() -> Bool {
        let success = undoHandler?() ?? false
        if success {
            revision &+= 1
            refresh()
        }
        return success
    }

    @discardableResult
    func redo() -> Bool {
        let success = redoHandler?() ?? false
        if success {
            revision &+= 1
            refresh()
        }
        return success
    }

    func refresh() {
        canUndo = canUndoHandler?() ?? false
        canRedo = canRedoHandler?() ?? false
    }
}

struct SketchHistoryToolbar: View {
    @ObservedObject var history: SketchHistoryController

    var body: some View {
        HStack(spacing: 6) {
            Button {
                history.undo()
            } label: {
                Label("Отменить", systemImage: "arrow.uturn.backward")
            }
            .keyboardShortcut("z", modifiers: .command)
            .disabled(!history.canUndo)

            Button {
                history.redo()
            } label: {
                Label("Повторить", systemImage: "arrow.uturn.forward")
            }
            .keyboardShortcut("z", modifiers: [.command, .shift])
            .disabled(!history.canRedo)
        }
    }
}
