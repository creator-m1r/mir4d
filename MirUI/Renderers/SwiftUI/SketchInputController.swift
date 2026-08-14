import SwiftUI

/// Translates pointer gestures from the sketch viewport into sketch commands.
/// Geometry remains owned by MirEngine; this controller only manages interaction state.
@MainActor
final class SketchInputController: ObservableObject {
    enum Phase {
        case idle
        case drawingLine(start: CGPoint)
    }

    @Published private(set) var phase: Phase = .idle
    @Published private(set) var cursor: CGPoint = .zero
    @Published private(set) var snap: SketchSnapUI?
    @Published private(set) var previewLine: (CGPoint, CGPoint)?

    var activeTool: SketchTool = .select
    weak var commandBridge: SketchCommandBridge?

    func pointerMoved(to point: CGPoint) {
        cursor = point

        guard case let .drawingLine(start) = phase else {
            previewLine = nil
            return
        }

        previewLine = (start, snappedPoint(point))
    }

    func pointerDown(at point: CGPoint) {
        let point = snappedPoint(point)

        switch activeTool {
        case .line:
            switch phase {
            case .idle:
                phase = .drawingLine(start: point)
                previewLine = (point, point)
            case let .drawingLine(start):
                previewLine = nil
                phase = .idle
                _ = commandBridge?.createLine(start: start, end: point)
            }
        default:
            break
        }
    }

    func cancel() {
        phase = .idle
        previewLine = nil
    }

    private func snappedPoint(_ point: CGPoint) -> CGPoint {
        // UI-side placeholder. The production bridge will query MirEngine's
        // SketchSnapEngine and return the transformed model-space point.
        snap = nil
        return point
    }
}

struct SketchSnapUI: Equatable {
    let title: String
    let point: CGPoint
}
