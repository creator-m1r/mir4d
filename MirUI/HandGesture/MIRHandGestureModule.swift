import Foundation
import simd
import Combine

/// Top-level facade for the MIR 4D hand-gesture subsystem.
///
/// It is the single, stable contract between the recognition engine and the
/// rest of the application (Spatial Menu, CAD). It deliberately never exposes
/// camera or recognition internals; consumers read the derived `spatialContext`
/// and receive `MIRHandIntent` through `MIRIntentRouter`.
@MainActor
public final class MIRHandGestureModule: ObservableObject {
    public static let shared = MIRHandGestureModule()

    @Published public private(set) var session = MIRHandTrackingSession()

    var configuration: MIRHandGestureConfiguration {
        get { session.configuration }
        set { session.configuration = newValue }
    }

    var status: MIRHandTrackingStatus { session.status }

    // MARK: - Lifecycle

    func startCamera() {
        session.start()
    }

    /// Test/diagnostic entry point: drive the pipeline from synthetic poses.
    func startMock(_ frames: [[MIRHandPose]], mode: MIRMockTrackingSource.Mode = .once) {
        session.startMock(frames, mode: mode)
    }

    func stop() {
        session.stop()
    }

    // MARK: - Spatial Menu interface

    /// The only state the Spatial Menu needs to read.
    var spatialContext: MIRHandSpatialContext {
        session.spatialContext()
    }

    /// Primary (first tracked) hand position in scene space, if available.
    var handPosition: SIMD3<Double>? {
        spatialContext.hands.first?.position
    }

    /// Primary hand pointing direction (from its active gesture), if available.
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

    // MARK: - Debug

    var debugInfo: MIRHandGestureDebugInfo? {
        session.debugInfo
    }

    func setDebugOverlay(enabled: Bool) {
        var config = configuration
        config.enableDebugOverlay = enabled
        configuration = config
    }
}
