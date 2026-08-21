import CoreGraphics

@MainActor
final class SketchCreationController {
    enum Tool {
        case line
        case circle
        case arc
    }

    private(set) var tool: Tool?
    private(set) var firstPoint: CGPoint?

    private let commitLine: ((CGPoint, CGPoint) -> Bool)?
    private let commitCircle: ((CGPoint, CGFloat) -> Bool)?
    private let commitArc: ((CGPoint, CGPoint, CGPoint) -> Bool)?

    init(
        commitLine: ((CGPoint, CGPoint) -> Bool)? = nil,
        commitCircle: ((CGPoint, CGFloat) -> Bool)? = nil,
        commitArc: ((CGPoint, CGPoint, CGPoint) -> Bool)? = nil
    ) {
        self.commitLine = commitLine
        self.commitCircle = commitCircle
        self.commitArc = commitArc
    }

    func begin(tool: Tool, at point: CGPoint) {
        self.tool = tool
        self.firstPoint = point
    }

    @discardableResult
    func finish(at point: CGPoint) -> Bool {
        guard let tool, let firstPoint else { return false }

        defer {
            self.tool = nil
            self.firstPoint = nil
        }

        switch tool {
        case .line:
            return commitLine?(firstPoint, point) ?? false

        case .circle:
            let radius = hypot(point.x - firstPoint.x, point.y - firstPoint.y)
            return commitCircle?(firstPoint, radius) ?? false

        case .arc:

            return commitArc?(firstPoint, firstPoint, point) ?? false
        }
    }

    func cancel() {
        tool = nil
        firstPoint = nil
    }
}
