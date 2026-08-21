import SwiftUI

struct SketchCoordinateSpace {
    var origin: CGPoint
    var pixelsPerUnit: CGFloat
    var flipY: Bool = true

    static let `default` = SketchCoordinateSpace(
        origin: .zero,
        pixelsPerUnit: 24
    )

    func modelToScreen(_ point: CGPoint) -> CGPoint {
        let y = flipY ? -point.y : point.y
        return CGPoint(
            x: origin.x + point.x * pixelsPerUnit,
            y: origin.y + y * pixelsPerUnit
        )
    }

    func screenToModel(_ point: CGPoint) -> CGPoint {
        let x = (point.x - origin.x) / pixelsPerUnit
        let rawY = (point.y - origin.y) / pixelsPerUnit
        let y = flipY ? -rawY : rawY
        return CGPoint(x: x, y: y)
    }

    func snappedModelPoint(_ point: CGPoint, grid: CGFloat) -> CGPoint {
        guard grid > 0 else { return point }
        return CGPoint(
            x: (point.x / grid).rounded() * grid,
            y: (point.y / grid).rounded() * grid
        )
    }
}
