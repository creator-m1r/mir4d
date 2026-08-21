
import SwiftUI

class AlignmentViewModel: ObservableObject {
    @Published var selectedIds: Set<Int64> = []

    func alignLeft()    { executeAlign("Left") }
    func alignCenterH() { executeAlign("CenterHorizontal") }
    func alignRight()   { executeAlign("Right") }
    func alignTop()     { executeAlign("Top") }
    func alignCenterV() { executeAlign("CenterVertical") }
    func alignBottom()  { executeAlign("Bottom") }
    func distributeH()  { executeAlign("DistributeHorizontal") }
    func distributeV()  { executeAlign("DistributeVertical") }

    private func executeAlign(_ strategy: String) {
        guard selectedIds.count >= 2 else { return }
        let ids = Array(selectedIds)
        ids.withUnsafeBufferPointer { ptr in
            guard let baseAddress = ptr.baseAddress else { return }
            MirUI_AlignWidgets(baseAddress, Int32(ids.count), strategy.cString(using: .utf8)!)
        }
        MirUI_RenderFrame()
    }
}

struct AlignmentToolbarView: View {
    @ObservedObject var vm: AlignmentViewModel

    var body: some View {
        HStack(spacing: 8) {
            Text("Align:")
                .font(.caption)
                .foregroundColor(.secondary)

            Button { vm.alignLeft() } label: { Image(systemName: "arrow.left.to.line") }
                .help("Выровнять по левому краю")
            Button { vm.alignCenterH() } label: { Image(systemName: "arrow.left.and.right") }
                .help("Выровнять по горизонтальному центру")
            Button { vm.alignRight() } label: { Image(systemName: "arrow.right.to.line") }
                .help("Выровнять по правому краю")

            Divider().frame(height: 20)

            Button { vm.alignTop() } label: { Image(systemName: "arrow.up.to.line") }
                .help("Выровнять по верхнему краю")
            Button { vm.alignCenterV() } label: { Image(systemName: "arrow.up.and.down") }
                .help("Выровнять по вертикальному центру")
            Button { vm.alignBottom() } label: { Image(systemName: "arrow.down.to.line") }
                .help("Выровнять по нижнему краю")

            Divider().frame(height: 20)

            Button { vm.distributeH() } label: { Image(systemName: "arrow.left.and.right.square") }
                .help("Распределить по горизонтали")
            Button { vm.distributeV() } label: { Image(systemName: "arrow.up.and.down.square") }
                .help("Распределить по вертикали")
        }
        .padding(8)
        .background(.ultraThinMaterial)
        .cornerRadius(8)
    }
}
