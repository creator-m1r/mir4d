import SwiftUI

struct SketchCoordinateOverlay: View {
    let cursor: CGPoint
    let size: CGSize
    let gridStep: CGFloat

    var body: some View {
        ZStack {
            cursorCrosshair
            coordinateReadout
            originMarker
        }
        .allowsHitTesting(false)
    }

    private var center: CGPoint {
        CGPoint(x: size.width / 2, y: size.height / 2)
    }

    private var modelX: Double {
        Double((cursor.x - center.x) / max(gridStep, 1))
    }

    private var modelY: Double {
        Double((center.y - cursor.y) / max(gridStep, 1))
    }

    private var cursorCrosshair: some View {
        Path { path in
            path.move(to: CGPoint(x: cursor.x - 12, y: cursor.y))
            path.addLine(to: CGPoint(x: cursor.x + 12, y: cursor.y))
            path.move(to: CGPoint(x: cursor.x, y: cursor.y - 12))
            path.addLine(to: CGPoint(x: cursor.x, y: cursor.y + 12))
        }
        .stroke(.cyan.opacity(0.8), lineWidth: 1)
    }

    private var coordinateReadout: some View {
        Text(String(format: "X %.2f  Y %.2f", modelX, modelY))
            .font(.system(size: 10, weight: .medium, design: .monospaced))
            .foregroundStyle(.white.opacity(0.9))
            .padding(.horizontal, 8)
            .padding(.vertical, 5)
            .background(.ultraThinMaterial)
            .clipShape(RoundedRectangle(cornerRadius: 6))
            .position(
                x: min(max(cursor.x + 72, 72), max(size.width - 72, 72)),
                y: min(max(cursor.y - 18, 18), max(size.height - 18, 18))
            )
    }

    private var originMarker: some View {
        ZStack {
            Circle()
                .stroke(.white.opacity(0.8), lineWidth: 1)
                .frame(width: 8, height: 8)
            Text("0")
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundStyle(.white.opacity(0.75))
                .offset(x: 10, y: 10)
        }
        .position(center)
    }
}
