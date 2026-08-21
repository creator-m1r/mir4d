import SwiftUI

/// Lightweight visual cursor for Sketch mode.
/// It is deliberately UI-only; model coordinates remain owned by MirEngine.
struct SketchCrosshairOverlay: View {
    let point: CGPoint
    let visible: Bool

    init(point: CGPoint, visible: Bool = true) {
        self.point = point
        self.visible = visible
    }

    var body: some View {
        if visible {
            ZStack(alignment: .topLeading) {
                Path { path in
                    path.move(to: CGPoint(x: point.x - 12, y: point.y))
                    path.addLine(to: CGPoint(x: point.x + 12, y: point.y))
                    path.move(to: CGPoint(x: point.x, y: point.y - 12))
                    path.addLine(to: CGPoint(x: point.x, y: point.y + 12))
                }
                .stroke(.cyan.opacity(0.85), lineWidth: 1)

                Circle()
                    .stroke(.cyan, lineWidth: 1.5)
                    .frame(width: 8, height: 8)
                    .position(point)

                Text(String(format: "X %.1f  Y %.1f", point.x, point.y))
                    .font(.system(size: 10, weight: .medium, design: .monospaced))
                    .foregroundStyle(.white.opacity(0.9))
                    .padding(.horizontal, 7)
                    .padding(.vertical, 4)
                    .background(.black.opacity(0.72))
                    .clipShape(RoundedRectangle(cornerRadius: 5))
                    .offset(x: point.x + 14, y: point.y + 14)
            }
            .allowsHitTesting(false)
        }
    }
}
