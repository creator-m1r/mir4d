import CoreGraphics
import Foundation

/// Rich, App-layer intent for a hand-drawn sketch stroke.
///
/// Built from the raw hand stream by the spatial-menu hand adapter while the
/// Sketch workbench is active. `points` are normalized interaction-volume
/// coordinates (-1…1, screen-centred, y up); the command bridge projects them
/// onto the active sketch plane.
struct MIR4DSketchIntent: Sendable {
    /// Stroke vertices in normalized interaction-volume space.
    let points: [CGPoint]
    /// True while the stroke is still being drawn (live preview frame); false
    /// on the committing frame.
    let live: Bool
}
