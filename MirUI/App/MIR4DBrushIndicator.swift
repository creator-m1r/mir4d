import SwiftUI
import Combine

/// Live, screen-space indicator for the air-sculpt brush.
///
/// The sculpt bridge publishes the normalized brush position (nx, ny in -1…1,
/// screen-centred, y up) and the current tool every frame. The workspace draws
/// a 2D overlay exactly where the hand points — which is the same screen point
/// the ray-cast brush lands under — so the user always sees where the
/// deformation will be applied.
@MainActor
final class MIR4DBrushIndicator: ObservableObject {
    static let shared = MIR4DBrushIndicator()

    /// Whether the indicator should be drawn (a sculpt stroke is active).
    @Published var active: Bool = false
    /// Normalized brush position, -1…1, screen-centred, y up.
    @Published var position: CGPoint = .zero
    /// Brush radius as a fraction of the object's half-size.
    @Published var radius: CGFloat = 0.2
    /// Current sculpt mode, for the on-screen label.
    @Published var mode: MIR4DSculptIntent.Mode? = nil

    func show(x: Double, y: Double, radius: CGFloat, mode: MIR4DSculptIntent.Mode) {
        active = true
        position = CGPoint(x: x, y: y)
        self.radius = radius
        self.mode = mode
    }

    func hide() {
        active = false
        mode = nil
    }
}
