import Foundation
import simd

/// Aggregate configuration for the whole hand-gesture module.
/// All tunables are collected here so consumers can adjust behaviour without
/// touching the recognition pipeline.
public struct MIRHandGestureConfiguration: Sendable {
    var recognizer: MIRHandGestureRecognizer.Configuration = .init()
    var mapper: MIRHandSpatialMapper = .init()
    var minimumIntentConfidence: Double = 0.55
    var enableDebugOverlay: Bool = false

    /// Режим визуализации скелета кистей (отдельный debug / assist режим,
    /// по умолчанию выключен; не влияет на CAD-геометрию и History).
    public var skeletonVisualizationMode: MIRHandSkeletonVisMode = .off

    // Стиль оверлея скелета (передаётся в движок через C-ABI).
    public var skeletonLeftColor = SIMD3<Double>(0.20, 0.90, 0.95)
    public var skeletonRightColor = SIMD3<Double>(1.00, 0.55, 0.15)
    public var skeletonJointSize: Double = 5.0
    public var skeletonTipSize: Double = 7.0
    public var skeletonWristSize: Double = 8.0
    public var skeletonAlpha: Double = 0.95
    /// Когда true — скелет участвует в depth-test (перекрывается CAD-геометрией).
    public var skeletonDepthTest: Bool = false

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
