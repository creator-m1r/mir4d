import SwiftUI

/// CAD-style crosshair and coordinate readout.
/// Rendering only; it does not own sketch geometry.
struct SketchCursorOverlay: View {
    let cursor: CGPoint
    let coordinateSpace: SketchCoordinateSpace
    let snap: SketchSnapUI?

    var body: some View {
        let modelPoint = coordinateSpace.screenToModel(cursor)

        ZStack(alignment: .topLeading) {
            Path { path in
                path.move(to: CGPoint(x: cursor.x - 18, y: cursor.y))
                path.addLine(to: CGPoint(x: cursor.x + 18, y: cursor.y))
                path.move(to: CGPoint(x: cursor.x, y: cursor.y - 18))
                path.addLine(to: CGPoint(x: cursor.x, y: cursor.y + 18))
            }
            .stroke(.cyan.opacity(0.9), lineWidth: 1)

            Circle()
                .stroke(.cyan, lineWidth: 1)
                .frame(width: 7, height: 7)
                .position(cursor)

            VStack(alignment: .leading, spacing: 2) {
                Text("X \(modelPoint.x, specifier: "%.3f")")
                Text("Y \(modelPoint.y, specifier: "%.3f")")
                if let snap {
                    Text(snap.title)
                        .fontWeight(.semibold)
                }
            }
            .font(.system(size: 10, design: .monospaced))
            .padding(6)
            .background(.black.opacity(0.72))
            .foregroundStyle(.white)
            .clipShape(RoundedRectangle(cornerRadius: 5))
            .offset(x: 22, y: 20)
        }
        .allowsHitTesting(false)
    }
}
