import Foundation

#if MIR4D_SWIFTPM
/// SwiftPM is a source-only UI smoke target; native camera control is exercised by Xcode.
public func Mir4DSetActiveCameraPreset(_ preset: MirCameraPreset) {
    _ = preset
}
#else
/// Requests a camera preset on the active viewport.
///
/// The C entry point `MirEngineSetActiveCameraPreset(void* viewport, int preset)`
/// requires the viewport handle, which lives in MirGLView. The sphere has no
/// viewport, so it publishes a notification; MirGLView observes it and forwards
/// the request to MirEngine with the correct handle.
public func Mir4DSetActiveCameraPreset(_ preset: MirCameraPreset) {
    NotificationCenter.default.post(
        name: .mir4DCameraPresetRequested,
        object: nil,
        userInfo: ["preset": preset.rawValue]
    )
}
#endif
