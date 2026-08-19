import SwiftUI
import MirUIHandGesture

/// Debug / assist toggle for the separate hand-skeleton 3D visualization mode.
/// По умолчанию выключен; не влияет на CAD-геометрию и History.
struct MIRHandSkeletonModeControl: View {
    @State private var mode: MIRHandSkeletonVisMode = .off

    var body: some View {
        Picker("Скелет кистей", selection: $mode) {
            Text("Выкл").tag(MIRHandSkeletonVisMode.off)
            Text("Суставы").tag(MIRHandSkeletonVisMode.jointsOnly)
            Text("Кости").tag(MIRHandSkeletonVisMode.bones)
            Text("Кости+Луч").tag(MIRHandSkeletonVisMode.bonesAndRays)
        }
        .pickerStyle(.segmented)
        .labelsHidden()
        .padding(6)
        .background(.black.opacity(0.5))
        .cornerRadius(8)
        .onChange(of: mode) { _, newMode in
            MIRHandGestureModule.shared.setSkeletonVisualizationMode(newMode)
        }
    }
}
