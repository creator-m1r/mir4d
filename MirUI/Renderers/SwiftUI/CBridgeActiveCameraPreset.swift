import Foundation

#if MIR4D_SWIFTPM

public func Mir4DSetActiveCameraPreset(_ preset: MirCameraPreset, animated: Bool = false) {
    _ = preset
}
#else

public func Mir4DSetActiveCameraPreset(_ preset: MirCameraPreset, animated: Bool = false) {
    NotificationCenter.default.post(
        name: .mir4DCameraPresetRequested,
        object: nil,
        userInfo: [
            "preset": String(preset.presetIndex),
            "animated": animated
        ]
    )
}
#endif
