import SwiftUI
import Combine

/// User-tunable air-sculpt brush parameters.
///
/// Exposed as on-screen sliders so the brush can be adjusted live instead of
/// being hard-coded. Both scales are relative to the selected object's
/// half-extent, keeping the brush proportional for any model size.
@MainActor
final class MIR4DSculptSettings: ObservableObject {
    static let shared = MIR4DSculptSettings()

    /// Brush radius as a fraction of the object half-extent (0.05…1).
    @Published var radiusScale: Double = 0.25
    /// Per-frame displacement as a fraction of the object half-extent at full
    /// pinch strength (0.005…0.2). Lower values keep strokes subtle.
    @Published var strengthScale: Double = 0.04
}
