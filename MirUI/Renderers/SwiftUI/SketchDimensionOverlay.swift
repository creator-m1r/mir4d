import SwiftUI

/// Visual annotation layer for sketch dimensions.
/// Values are presentation-only until connected to MirEngine constraint data.
struct SketchDimensionOverlay: View {
    let start: CGPoint
    let end: CGPoint
    let value: String
    var color: Color = .yellow

    var body: some View {
        ZStack {
            dimensionLine
            extensionLines
            label
        }
        .allowsHitTesting(false)
    }

    private var midpoint: CGPoint {
        CGPoint(x: (start.x + end.x) / 2, y: (start.y + end.y) / 2)
    }

    private var length: CGFloat {
        hypot(end.x - start.x, end.y - start.y)
    }

    private var angle: Angle {
        .radians(Double(atan2(end.y - start.y, end.x - start.x)))
    }

    private var dimensionLine: some View {
        Path { path in
            path.move(to: start)
            path.addLine(to: end)
        }
        .stroke(color.opacity(0.8), style: StrokeStyle(lineWidth: 1, dash: [4, 3]))
    }

    private var extensionLines: some View {
        Path { path in
            path.move(to: CGPoint(x: start.x, y: start.y - 8))
            path.addLine(to: CGPoint(x: start.x, y: start.y + 8))
            path.move(to: CGPoint(x: end.x, y: end.y - 8))
            path.addLine(to: CGPoint(x: end.x, y: end.y + 8))
        }
        .stroke(color.opacity(0.55), lineWidth: 1)
    }

    private var label: some View {
        Text(value)
            .font(.system(size: 10, weight: .semibold, design: .monospaced))
            .foregroundStyle(color)
            .padding(.horizontal, 6)
            .padding(.vertical, 3)
            .background(.ultraThinMaterial)
            .clipShape(RoundedRectangle(cornerRadius: 5))
            .position(x: midpoint.x, y: midpoint.y - 12)
    }
}
