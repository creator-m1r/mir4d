import SwiftUI

struct SketchViewportView: View {
    @StateObject private var controller = SketchInputController()
    @StateObject private var history = SketchHistoryController()
    @StateObject private var navigation = SketchViewportNavigation()
    @State private var showGrid = true
    @State private var magnification: CGFloat = 1.0

    var body: some View {
        VStack(spacing: 0) {
            SketchHistoryToolbar(history: history)
                .padding(6)

            Divider()

            GeometryReader { proxy in
                let coordinateSpace = SketchCoordinateSpace(
                    origin: CGPoint(
                        x: proxy.size.width / 2 + navigation.pan.width,
                        y: proxy.size.height / 2 + navigation.pan.height
                    ),
                    pixelsPerUnit: 24 * navigation.zoom
                )

                ZStack {
                    Color.black

                    if showGrid {
                        SketchInfiniteGridView(
                            zoom: navigation.zoom,
                            pan: navigation.pan
                        )
                    }

                    sketchAxes(in: proxy.size)
                    sketchOrigin
                    sketchAxisLabels(in: proxy.size)
                    preview
                    SketchSnapOverlay(snap: controller.snap)
                    SketchCursorOverlay(
                        cursor: controller.cursor,
                        coordinateSpace: coordinateSpace,
                        snap: controller.snap
                    )
                }
                .contentShape(Rectangle())
                .gesture(
                    DragGesture(minimumDistance: 0)
                        .onChanged { value in
                            controller.pointerMoved(to: value.location)
                        }
                        .onEnded { value in
                            controller.pointerDown(at: value.location)
                        }
                )
                .simultaneousGesture(
                    MagnifyGesture()
                        .onChanged { value in
                            navigation.magnifyChanged(
                                value.magnification,
                                around: controller.cursor,
                                previousMagnification: &magnification
                            )
                        }
                        .onEnded { _ in
                            magnification = 1.0
                        }
                )
                .onAppear {
                    controller.activeTool = .line
                    controller.pointerMoved(
                        to: CGPoint(x: proxy.size.width / 2, y: proxy.size.height / 2)
                    )
                }
                .onKeyPress(.escape) {
                    controller.cancel()
                    return .handled
                }
                .overlay(alignment: .topTrailing) {
                    viewportControls
                        .padding(8)
                }
            }
        }
    }

    @ViewBuilder
    private var preview: some View {
        if let line = controller.previewLine {
            Path { path in
                path.move(to: line.0)
                path.addLine(to: line.1)
            }
            .stroke(.cyan, style: StrokeStyle(lineWidth: 2, dash: [7, 5]))
        }
    }

    private var sketchOrigin: some View {
        GeometryReader { proxy in
            let center = CGPoint(
                x: proxy.size.width / 2 + navigation.pan.width,
                y: proxy.size.height / 2 + navigation.pan.height
            )

            ZStack {
                Circle()
                    .fill(.white)
                    .frame(width: 6, height: 6)
                    .position(center)

                Circle()
                    .stroke(.white.opacity(0.7), lineWidth: 1)
                    .frame(width: 14, height: 14)
                    .position(center)
            }
            .allowsHitTesting(false)
        }
    }

    private func sketchAxes(in size: CGSize) -> some View {
        let center = CGPoint(
            x: size.width / 2 + navigation.pan.width,
            y: size.height / 2 + navigation.pan.height
        )

        return Path { path in
            path.move(to: CGPoint(x: 0, y: center.y))
            path.addLine(to: CGPoint(x: size.width, y: center.y))
            path.move(to: CGPoint(x: center.x, y: 0))
            path.addLine(to: CGPoint(x: center.x, y: size.height))
        }
        .stroke(.gray.opacity(0.65), lineWidth: 1)
        .allowsHitTesting(false)
    }

    private func sketchAxisLabels(in size: CGSize) -> some View {
        let center = CGPoint(
            x: size.width / 2 + navigation.pan.width,
            y: size.height / 2 + navigation.pan.height
        )

        return ZStack {
            Text("X")
                .font(.system(size: 11, weight: .bold, design: .monospaced))
                .foregroundStyle(.red.opacity(0.9))
                .position(x: min(max(center.x + 18, 16), size.width - 16), y: min(max(center.y - 14, 12), size.height - 12))

            Text("Y")
                .font(.system(size: 11, weight: .bold, design: .monospaced))
                .foregroundStyle(.green.opacity(0.9))
                .position(x: min(max(center.x + 14, 12), size.width - 12), y: min(max(center.y - 22, 12), size.height - 12))
        }
        .allowsHitTesting(false)
    }

    private var viewportControls: some View {
        HStack(spacing: 8) {
            Text("\(Int(navigation.zoom * 100))%")
                .font(.system(size: 10, design: .monospaced))
                .foregroundStyle(.secondary)

            Button {
                navigation.zoom(by: 1.2, around: controller.cursor)
            } label: {
                Image(systemName: "plus.magnifyingglass")
            }
            .buttonStyle(.borderless)

            Button {
                navigation.zoom(by: 1.0 / 1.2, around: controller.cursor)
            } label: {
                Image(systemName: "minus.magnifyingglass")
            }
            .buttonStyle(.borderless)

            Button {
                navigation.reset()
            } label: {
                Image(systemName: "scope")
            }
            .buttonStyle(.borderless)
            .help("Сбросить вид")
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 7))
    }
}

#Preview {
    SketchViewportView()
        .frame(width: 900, height: 600)
}
