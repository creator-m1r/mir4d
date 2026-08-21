import Foundation

/// Requests an arbitrary camera orbit (azimuth theta, polar angle phi,
/// distance) on the active viewport. The request is delivered through the
/// Event Bus to MirGLCustomView, which applies it to MirEngine.
/// The engine state is not touched directly from the view layer.
public func Mir4DSetCameraOrbit(theta: Double, phi: Double, distance: Double, animated: Bool = true) {
    NotificationCenter.default.post(
        name: .mir4DCameraOrbitRequested,
        object: nil,
        userInfo: [
            "theta": theta,
            "phi": phi,
            "distance": distance,
            "animated": animated
        ]
    )
}

/// Requests "Fit All" on the active viewport (same Event Bus path).
public func Mir4DRequestCameraFit() {
    NotificationCenter.default.post(
        name: .mir4DFitViewport,
        object: nil
    )
}
