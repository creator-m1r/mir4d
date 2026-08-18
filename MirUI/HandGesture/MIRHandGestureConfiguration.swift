import Foundation
import simd

/// Aggregate configuration for the whole hand-gesture module.
/// All tunables are collected here so consumers can adjust behaviour without
/// touching the recognition pipeline.
struct MIRHandGestureConfiguration: Sendable {
    var recognizer: MIRHandGestureRecognizer.Configuration = .init()
    var mapper: MIRHandSpatialMapper = .init()
    var minimumIntentConfidence: Double = 0.55
    var enableDebugOverlay: Bool = false

    /// Whether two-hand relational gestures are evaluated.
    var twoHandEnabled: Bool = true

    /// Dead-zones for two-hand gestures (in scene-space units).
    struct TwoHand: Sendable {
        var minimumHandDistance: Double = 0.08
        var scaleDeadZone: Double = 0.015
        var rotationDeadZone: Double = 0.025
        var translationDeadZone: Double = 0.008
    }

    var twoHand: TwoHand = .init()
}
