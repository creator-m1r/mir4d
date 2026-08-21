import Foundation
import CoreGraphics
import Combine

/// Bridges a hand-drawn sketch stroke onto the active sketch plane.
///
/// The raw hand stream works in a normalized interaction volume; the sketch
/// plane lives in world space. This bridge maps the stroke vertices onto the
/// plane (XY base plane by default) and pushes them as an immediate 2D sketch
/// overlay through the renderer. Multiple strokes accumulate into a single
/// committed overlay; `live` frames provide a real-time preview while drawing.
@MainActor
final class MIR4DSketchCommandBridge {
    static let shared = MIR4DSketchCommandBridge()

    /// Half-extent (mm) the hand interaction volume maps onto the sketch plane,
    /// so a full-width hand sweep draws a 200 mm wide sketch.
    private let planeHalf: Double = 100
    private let strokeColor: (Float, Float, Float) = (0.2, 0.8, 1.0)

    private var committed: [MirEngineSketchSegment] = []
    private var cancellable: AnyCancellable?

    private init() {
        cancellable = MIR4DSketchIntentPublisher.shared.stream
            .sink { [weak self] intent in
                self?.apply(intent)
            }
    }

    private func apply(_ intent: MIR4DSketchIntent) {
        guard intent.points.count >= 2 else {
            // A degenerate stroke clears the overlay unless it is a live preview.
            if !intent.live { committed.removeAll() }
            MirEnginePushSketch(MIR4DModelRuntime.shared.renderer, intent.live ? committed : [])
            return
        }

        var segs: [MirEngineSketchSegment] = []
        for i in 1..<intent.points.count {
            let a = intent.points[i - 1]
            let b = intent.points[i]
            segs.append(MirEngineSketchSegment(
                ax: Float(a.x * planeHalf), ay: Float(a.y * planeHalf),
                bx: Float(b.x * planeHalf), by: Float(b.y * planeHalf),
                color: strokeColor
            ))
        }

        if intent.live {
            push(committed + segs)
        } else {
            committed.append(contentsOf: segs)
            push(committed)
        }
    }

    /// Pushes segments onto the active sketch plane (XY/YZ/ZX), defaulting to the
    /// XY base plane when no plane is chosen.
    private func push(_ segments: [MirEngineSketchSegment]) {
        let plane = MIR4DModelRuntime.shared.activeSketchPlane
        let origin: [Float] = plane.map { [Float($0.origin.x), Float($0.origin.y), Float($0.origin.z)] } ?? [0, 0, 0]
        let xAxis: [Float] = plane.map { [Float($0.xAxis.x), Float($0.xAxis.y), Float($0.xAxis.z)] } ?? [1, 0, 0]
        let yAxis: [Float] = plane.map { [Float($0.yAxis.x), Float($0.yAxis.y), Float($0.yAxis.z)] } ?? [0, 1, 0]
        MirEnginePushSketch(MIR4DModelRuntime.shared.renderer, segments, origin: origin, xAxis: xAxis, yAxis: yAxis)
    }
}
