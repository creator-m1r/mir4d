import Foundation
import simd
import Combine

@MainActor
public final class MIRHandGestureModule: ObservableObject {
    public static let shared = MIRHandGestureModule()

    @Published public private(set) var session = MIRHandTrackingSession()

    public var configuration: MIRHandGestureConfiguration {
        get { session.configuration }
        set { session.configuration = newValue }
    }

    var status: MIRHandTrackingStatus { session.status }

    public func startCamera() {
        session.start()
    }

    func startMock(_ frames: [[MIRHandPose]], mode: MIRMockTrackingSource.Mode = .once) {
        session.startMock(frames, mode: mode)
    }

    public func stop() {
        session.stop()
    }

    var spatialContext: MIRHandSpatialContext {
        session.spatialContext()
    }

    var handPosition: SIMD3<Double>? {
        spatialContext.hands.first?.position
    }

    var handDirection: SIMD3<Double>? {
        spatialContext.hands.first?.direction
    }

    var primaryGesture: MIRHandGestureType? {
        spatialContext.hands.first?.gesture
    }

    var primaryPinch: Double? {
        spatialContext.hands.first?.pinch
    }

    var twoHandGesture: MIRHandGestureType {
        spatialContext.twoHandGesture
    }

    var debugInfo: MIRHandGestureDebugInfo? {
        session.debugInfo
    }

    func setDebugOverlay(enabled: Bool) {
        var config = configuration
        config.enableDebugOverlay = enabled
        configuration = config
    }

    public func setSkeletonVisualizationMode(_ mode: MIRHandSkeletonVisMode) {
        var config = configuration
        config.skeletonVisualizationMode = mode
        configuration = config
    }
}
