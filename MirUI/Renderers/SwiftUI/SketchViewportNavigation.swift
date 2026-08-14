import SwiftUI

/// Presentation-only navigation state for the sketch viewport.
/// The sketch model remains independent from camera/pan/zoom state.
@MainActor
final class SketchViewportNavigation: ObservableObject {
    @Published var zoom: CGFloat = 1.0
    @Published var pan: CGSize = .zero

    let minimumZoom: CGFloat = 0.2
    let maximumZoom: CGFloat = 8.0

    func magnifyChanged(_ magnification: CGFloat, around point: CGPoint, previousMagnification: inout CGFloat) {
        guard magnification > 0 else { return }
        let incrementalFactor = magnification / previousMagnification
        previousMagnification = magnification
        zoom(by: incrementalFactor, around: point)
    }

    func zoom(by factor: CGFloat, around point: CGPoint? = nil) {
        guard factor.isFinite, factor > 0 else { return }

        let oldZoom = zoom
        let newZoom = min(max(oldZoom * factor, minimumZoom), maximumZoom)
        guard newZoom != oldZoom else { return }

        if let point {
            let ratio = newZoom / oldZoom
            pan = CGSize(
                width: point.x - (point.x - pan.width) * ratio,
                height: point.y - (point.y - pan.height) * ratio
            )
        }

        zoom = newZoom
    }

    func reset() {
        zoom = 1.0
        pan = .zero
    }
}
