import SwiftUI

/// Pure interaction layer for the Sketch workbench.
/// Keeps pointer/navigation rules separate from SwiftUI rendering.
struct SketchInteractionController {
    var zoom: CGFloat = 1
    var pan: CGSize = .zero
    var gridStep: CGFloat = 10
    var snapEnabled = true

    mutating func zoom(by delta: CGFloat, around screenPoint: CGPoint, canvasSize: CGSize) {
        let oldZoom = zoom
        zoom = min(max(0.15, zoom * delta), 20)
        guard oldZoom != zoom else { return }

        let factor = zoom / oldZoom
        let center = CGPoint(x: canvasSize.width / 2, y: canvasSize.height / 2)
        let dx = screenPoint.x - center.x - pan.width
        let dy = screenPoint.y - center.y - pan.height
        pan = CGSize(
            width: screenPoint.x - center.x - dx * factor,
            height: screenPoint.y - center.y - dy * factor
        )
    }

    mutating func panBy(_ delta: CGSize) {
        pan.width += delta.width
        pan.height += delta.height
    }

    func worldPoint(from screenPoint: CGPoint, canvasSize: CGSize) -> CGPoint {
        CGPoint(
            x: (screenPoint.x - canvasSize.width / 2 - pan.width) / zoom,
            y: -(screenPoint.y - canvasSize.height / 2 - pan.height) / zoom
        )
    }

    func screenPoint(from worldPoint: CGPoint, canvasSize: CGSize) -> CGPoint {
        CGPoint(
            x: canvasSize.width / 2 + worldPoint.x * zoom + pan.width,
            y: canvasSize.height / 2 - worldPoint.y * zoom + pan.height
        )
    }

    func snapped(_ point: CGPoint) -> CGPoint {
        guard snapEnabled, gridStep > 0 else { return point }
        return CGPoint(
            x: (point.x / gridStep).rounded() * gridStep,
            y: (point.y / gridStep).rounded() * gridStep
        )
    }

    func distance(_ a: CGPoint, _ b: CGPoint) -> CGFloat {
        hypot(b.x - a.x, b.y - a.y)
    }
}
