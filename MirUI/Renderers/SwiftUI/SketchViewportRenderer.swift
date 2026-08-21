import SwiftUI

struct SketchViewportRenderer: View {
    struct Line: Identifiable, Equatable {
        let id: UInt32
        let start: CGPoint
        let end: CGPoint
    }

    let lines: [Line]
    let selectedIDs: Set<UInt32>
    var endpointRadius: CGFloat = 4
    var selectedLineWidth: CGFloat = 3
    var lineWidth: CGFloat = 1.5
    var onSelect: ((UInt32, Bool) -> Void)?
    var onEndpointDrag: ((UInt32, Bool, CGPoint) -> Void)?

    var body: some View {
        GeometryReader { proxy in
            Canvas { context, _ in
                for line in lines {
                    var path = Path()
                    path.move(to: line.start)
                    path.addLine(to: line.end)

                    context.stroke(
                        path,
                        with: .color(selectedIDs.contains(line.id) ? .accentColor : .primary),
                        lineWidth: selectedIDs.contains(line.id) ? selectedLineWidth : lineWidth
                    )

                    if selectedIDs.contains(line.id) {
                        drawEndpoint(context, at: line.start)
                        drawEndpoint(context, at: line.end)
                    }
                }
            }
            .contentShape(Rectangle())
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onEnded { value in
                        handleClick(at: value.location)
                    }
            )
            .overlay(alignment: .topLeading) {
                endpointHandles(in: proxy.size)
            }
        }
    }

    @ViewBuilder
    private func endpointHandles(in size: CGSize) -> some View {
        ForEach(lines.filter { selectedIDs.contains($0.id) }) { line in
            EndpointHandle(
                point: line.start,
                radius: endpointRadius,
                onDrag: { point in
                    onEndpointDrag?(line.id, true, point)
                }
            )

            EndpointHandle(
                point: line.end,
                radius: endpointRadius,
                onDrag: { point in
                    onEndpointDrag?(line.id, false, point)
                }
            )
        }
    }

    private func drawEndpoint(_ context: GraphicsContext, at point: CGPoint) {
        let rect = CGRect(
            x: point.x - endpointRadius,
            y: point.y - endpointRadius,
            width: endpointRadius * 2,
            height: endpointRadius * 2
        )
        context.fill(Path(ellipseIn: rect), with: .color(.accentColor))
    }

    private func handleClick(at point: CGPoint) {
        guard let hit = nearestLine(to: point) else {
            return
        }

        onSelect?(hit.id, false)
    }

    private func nearestLine(to point: CGPoint) -> Line? {
        var best: (line: Line, distance: CGFloat)?

        for line in lines {
            let distance = pointToSegmentDistance(point, line.start, line.end)
            guard distance <= 10 else { continue }

            if best == nil || distance < best!.distance {
                best = (line, distance)
            }
        }

        return best?.line
    }

    private func pointToSegmentDistance(
        _ point: CGPoint,
        _ a: CGPoint,
        _ b: CGPoint
    ) -> CGFloat {
        let ab = CGPoint(x: b.x - a.x, y: b.y - a.y)
        let ap = CGPoint(x: point.x - a.x, y: point.y - a.y)
        let lengthSquared = ab.x * ab.x + ab.y * ab.y

        guard lengthSquared > 0.000001 else {
            return hypot(point.x - a.x, point.y - a.y)
        }

        let t = max(0, min(1, (ap.x * ab.x + ap.y * ab.y) / lengthSquared))
        let closest = CGPoint(x: a.x + ab.x * t, y: a.y + ab.y * t)
        return hypot(point.x - closest.x, point.y - closest.y)
    }
}

private struct EndpointHandle: View {
    let point: CGPoint
    let radius: CGFloat
    let onDrag: (CGPoint) -> Void

    var body: some View {
        Circle()
            .fill(Color.accentColor)
            .frame(width: radius * 2.5, height: radius * 2.5)
            .position(point)
            .gesture(
                DragGesture()
                    .onChanged { value in
                        onDrag(value.location)
                    }
            )
    }
}
