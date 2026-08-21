
import SwiftUI
#if os(macOS)
import AppKit
#endif

@MainActor
final class DesignerCanvasViewModel: ObservableObject {
    @ObservedObject var bridge = MirUIBridge.shared
    @Published var selectedWidgetIds: Set<Int64> = []
    @Published var isDragging = false
    @Published var isResizing = false
    @Published var resizeCorner: HandleCorner?
    @Published var isRubberBandActive = false
    @Published var rubberBandRect: CGRect = .zero
    @Published var activeGuidesX: [CGFloat] = []
    @Published var activeGuidesY: [CGFloat] = []

    enum HandleCorner: CaseIterable { case topLeft, topRight, bottomLeft, bottomRight
        var isRight: Bool { self == .topRight || self == .bottomRight }
        var isBottom: Bool { self == .bottomLeft || self == .bottomRight }
    }

    func selectWidget(id: Int64, shiftPressed: Bool = false) {
        if shiftPressed {
            if selectedWidgetIds.contains(id) { selectedWidgetIds.remove(id) } else { selectedWidgetIds.insert(id) }
        } else { selectedWidgetIds = [id] }
    }
    func clearSelection() { selectedWidgetIds = [] }
    func moveWidgets(ids: Set<Int64>, deltaX: Double, deltaY: Double) {
        for id in ids { MirUI_MoveWidget(id, deltaX, deltaY) }
        MirUI_RenderFrame()
    }
    func resizeWidget(id: Int64, newWidth: Double, newHeight: Double) {
        guard let node = bridge.nodes.first(where: { $0.widgetId == id }) else { return }
        MirUI_ResizeWidget(id, newWidth, newHeight, node.x, node.y)
        MirUI_RenderFrame()
    }
    func deleteSelected() {
        for id in selectedWidgetIds { MirUI_DeleteWidget(id) }
        clearSelection(); MirUI_RenderFrame()
    }
    func copySelected() { guard let id = selectedWidgetIds.first else { return }; MirUI_CopyWidget(id) }
    func paste() { MirUI_PasteWidget(0); MirUI_RenderFrame() }
    func cutSelected() { guard let id = selectedWidgetIds.first else { return }; MirUI_CutWidget(id); clearSelection(); MirUI_RenderFrame() }
    func selectWidgetsInRect(_ rect: CGRect) {
        selectedWidgetIds = Set(bridge.nodes.compactMap { node in
            let widgetRect = CGRect(x: node.x, y: node.y, width: node.width, height: node.height)
            return widgetRect.intersects(rect) ? node.widgetId : nil
        })
    }
    func undo() { MirUI_Undo(); MirUI_RenderFrame(); clearSelection() }
    func redo() { MirUI_Redo(); MirUI_RenderFrame(); clearSelection() }
}

struct DesignerCanvasView: View {
    @StateObject private var vm = DesignerCanvasViewModel()
    @StateObject private var alignVM = AlignmentViewModel()

    var body: some View {
        ZStack(alignment: .topLeading) {
            Color.white
            GridBackground(gridSize: 20)
            ForEach(vm.activeGuidesX, id: \.self) { x in Rectangle().fill(Color.blue.opacity(0.7)).frame(width: 1).position(x: x, y: 300) }
            ForEach(vm.activeGuidesY, id: \.self) { y in Rectangle().fill(Color.blue.opacity(0.7)).frame(height: 1).position(x: 400, y: y) }

            ForEach(vm.bridge.nodes, id: \.widgetId) { node in
                if node.visible {
                    WidgetEditOverlay(
                        node: node,
                        isSelected: vm.selectedWidgetIds.contains(node.widgetId),
                        onTap: { vm.selectWidget(id: node.widgetId, shiftPressed: NSEvent.modifierFlags.contains(.shift)) },
                        onDragChanged: { _ in vm.isDragging = true },
                        onDragEnd: { translation in
                            vm.isDragging = false
                            let ids = vm.selectedWidgetIds.contains(node.widgetId) ? vm.selectedWidgetIds : [node.widgetId]
                            vm.moveWidgets(ids: ids, deltaX: Double(translation.width), deltaY: Double(translation.height))
                            vm.activeGuidesX = []; vm.activeGuidesY = []
                        },
                        onResizeEnd: { newSize in vm.resizeWidget(id: node.widgetId, newWidth: Double(newSize.width), newHeight: Double(newSize.height)) }
                    )
                }
            }

            if vm.isRubberBandActive {
                Rectangle().stroke(Color.blue, style: StrokeStyle(lineWidth: 1, dash: [4]))
                    .background(Color.blue.opacity(0.1))
                    .frame(width: vm.rubberBandRect.width, height: vm.rubberBandRect.height)
                    .position(x: vm.rubberBandRect.midX, y: vm.rubberBandRect.midY)
            }
            if vm.selectedWidgetIds.count >= 2 { VStack { Spacer(); AlignmentToolbarView(vm: alignVM).padding(.bottom, 20) } }
            #if os(macOS)
            DesignerKeyCapture { event in
                guard event.modifierFlags.contains(.command) else {
                    if event.keyCode == 51 || event.keyCode == 117 { vm.deleteSelected() }
                    return
                }
                switch event.keyCode {
                case 8: vm.copySelected()
                case 9: vm.paste()
                case 7: vm.cutSelected()
                case 6: vm.undo()
                case 16: vm.redo()
                default: break
                }
            }.frame(width: 1, height: 1)
            #endif
        }
        .gesture(
            DragGesture(minimumDistance: 5)
                .onChanged { value in
                    if !vm.isRubberBandActive { vm.isRubberBandActive = true; vm.rubberBandRect = .zero }
                    let start = value.startLocation, current = value.location
                    vm.rubberBandRect = CGRect(x: min(start.x, current.x), y: min(start.y, current.y), width: abs(current.x - start.x), height: abs(current.y - start.y))
                }
                .onEnded { _ in
                    if vm.isRubberBandActive { vm.selectWidgetsInRect(vm.rubberBandRect); vm.isRubberBandActive = false }
                }
        )
        .onTapGesture { vm.clearSelection() }
    }
}

private struct GridBackground: View {
    let gridSize: CGFloat
    var body: some View {
        Canvas { context, size in
            let step = max(gridSize, 2)
            var path = Path()
            var x: CGFloat = 0
            while x <= size.width { path.move(to: CGPoint(x: x, y: 0)); path.addLine(to: CGPoint(x: x, y: size.height)); x += step }
            var y: CGFloat = 0
            while y <= size.height { path.move(to: CGPoint(x: 0, y: y)); path.addLine(to: CGPoint(x: size.width, y: y)); y += step }
            context.stroke(path, with: .color(Color.black.opacity(0.08)), lineWidth: 0.5)
        }.allowsHitTesting(false)
    }
}

private struct WidgetEditOverlay: View {
    let node: ViewNode
    let isSelected: Bool
    let onTap: () -> Void
    let onDragChanged: (DragGesture.Value) -> Void
    let onDragEnd: (CGSize) -> Void
    let onResizeEnd: (CGSize) -> Void

    var body: some View {
        ZStack(alignment: .bottomTrailing) {
            RoundedRectangle(cornerRadius: 4)
                .fill(Color.white.opacity(0.92))
                .overlay(Text(node.text).font(.system(size: 10)).foregroundStyle(.secondary).padding(4), alignment: .topLeading)
                .overlay(RoundedRectangle(cornerRadius: 4).stroke(isSelected ? Color.accentColor : Color.gray.opacity(0.35), lineWidth: isSelected ? 2 : 1))
                .frame(width: max(CGFloat(node.width), 20), height: max(CGFloat(node.height), 20))
                .contentShape(Rectangle())
                .onTapGesture(perform: onTap)
                .gesture(DragGesture().onChanged(onDragChanged).onEnded { onDragEnd($0.translation) })
            if isSelected {
                Circle().fill(Color.accentColor).frame(width: 10, height: 10).padding(2)
                    .gesture(DragGesture().onEnded { onResizeEnd($0.translation) })
            }
        }
        .position(x: CGFloat(node.x) + CGFloat(node.width) / 2, y: CGFloat(node.y) + CGFloat(node.height) / 2)
    }
}

#if os(macOS)
private struct DesignerKeyCapture: NSViewRepresentable {
    let onKeyDown: (NSEvent) -> Void
    func makeNSView(context: Context) -> KeyView { KeyView(onKeyDown: onKeyDown) }
    func updateNSView(_ nsView: KeyView, context: Context) { nsView.onKeyDown = onKeyDown }
    final class KeyView: NSView {
        var onKeyDown: (NSEvent) -> Void
        init(onKeyDown: @escaping (NSEvent) -> Void) { self.onKeyDown = onKeyDown; super.init(frame: .zero); wantsLayer = true; layer?.backgroundColor = NSColor.clear.cgColor }
        required init?(coder: NSCoder) { fatalError("init(coder:) has not been implemented") }
        override var acceptsFirstResponder: Bool { true }
        override func viewDidMoveToWindow() { super.viewDidMoveToWindow(); window?.makeFirstResponder(self) }
        override func keyDown(with event: NSEvent) { onKeyDown(event) }
    }
}
#endif
