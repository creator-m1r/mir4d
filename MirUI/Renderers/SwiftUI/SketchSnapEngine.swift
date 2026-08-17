import Foundation
import CoreGraphics

/// Fast UI-side snapping/inference helper.
/// MirEngine remains the authoritative geometry solver.
struct SketchSnapEngine {

    enum Kind: Equatable {
        case grid
        case endpoint
        case midpoint
        case center
        case intersection
    }

    struct Result: Equatable {
        let point: CGPoint
        let kind: Kind
        let distance: CGFloat
    }

    var gridStep: CGFloat = 10
    var tolerance: CGFloat = 12
    var enabled: Bool = true

    func snap(
        _ point: CGPoint,
        candidates: [CGPoint] = []
    ) -> Result {

        guard enabled else {
            return Result(
                point: point,
                kind: .grid,
                distance: 0
            )
        }

        // IMPORTANT:
        // Keep `best` as SketchSnapEngine.Result.
        // `grid(point)` itself returns CGPoint.
        let gridPoint = grid(point)

        var best = Result(
            point: gridPoint,
            kind: .grid,
            distance: distance(point, gridPoint)
        )

        for candidate in candidates {
            let d = distance(point, candidate)

            if d <= tolerance && d < best.distance {
                best = Result(
                    point: candidate,
                    kind: .endpoint,
                    distance: d
                )
            }
        }

        return best
    }

    private func grid(_ p: CGPoint) -> CGPoint {
        guard gridStep > 0 else {
            return p
        }

        return CGPoint(
            x: (p.x / gridStep).rounded() * gridStep,
            y: (p.y / gridStep).rounded() * gridStep
        )
    }

    private func distance(
        _ a: CGPoint,
        _ b: CGPoint
    ) -> CGFloat {
        hypot(
            b.x - a.x,
            b.y - a.y
        )
    }
}

/// Legacy UI presentation model for snap feedback used by the standalone
/// overlay views. MirEngine remains the authoritative geometry solver.
struct SketchSnapUI {
    let point: CGPoint
    let title: String
}
