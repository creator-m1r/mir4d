import Foundation
import simd

public struct MIRHandGestureConfiguration: Sendable {
    var recognizer: MIRHandGestureRecognizer.Configuration = .init()
    var mapper: MIRHandSpatialMapper = .init()
    var minimumIntentConfidence: Double = 0.55
    var enableDebugOverlay: Bool = false

    public var skeletonVisualizationMode: MIRHandSkeletonVisMode = .off

    public var skeletonLeftColor = SIMD3<Double>(0.20, 0.90, 0.95)
    public var skeletonRightColor = SIMD3<Double>(1.00, 0.55, 0.15)
    public var skeletonJointSize: Double = 5.0
    public var skeletonTipSize: Double = 7.0
    public var skeletonWristSize: Double = 8.0
    public var skeletonAlpha: Double = 0.95

    public var skeletonDepthTest: Bool = false

    var twoHandEnabled: Bool = true

    struct TwoHand: Sendable {
        var minimumHandDistance: Double = 0.08
        var scaleDeadZone: Double = 0.015
        var rotationDeadZone: Double = 0.025
        var translationDeadZone: Double = 0.008
    }

    var twoHand: TwoHand = .init()
}
