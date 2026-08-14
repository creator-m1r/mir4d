import Foundation

#if MIR4D_SWIFTPM
/// SwiftPM is a source-only UI smoke target; native camera control is exercised by Xcode.
public func Mir4DSetActiveCameraPreset(_ preset: MirCameraPreset) {
    _ = preset
}
#else
@_silgen_name("MirEngineSetActiveCameraPreset") private func MirEngineSetActiveCameraPresetNative(_ preset: Int32)

public func Mir4DSetActiveCameraPreset(_ preset: MirCameraPreset) {
    let value: Int32
    switch preset {
    case .front: value = 0
    case .back: value = 1
    case .left: value = 2
    case .right: value = 3
    case .top: value = 4
    case .bottom: value = 5
    case .isometric: value = 6
    }
    MirEngineSetActiveCameraPresetNative(value)
}
#endif
