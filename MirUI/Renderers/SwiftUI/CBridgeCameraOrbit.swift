import Foundation

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

public func Mir4DRequestCameraFit() {
    NotificationCenter.default.post(
        name: .mir4DFitViewport,
        object: nil
    )
}
