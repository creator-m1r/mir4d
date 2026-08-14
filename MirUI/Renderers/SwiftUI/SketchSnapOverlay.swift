import SwiftUI

/// Visual feedback for the active sketch snap candidate.
struct SketchSnapOverlay: View {
    let snap: SketchSnapUI?

    var body: some View {
        if let snap {
            ZStack {
                Circle()
                    .stroke(.orange, lineWidth: 2)
                    .frame(width: 14, height: 14)
                    .position(snap.point)

                Text(snap.title)
                    .font(.caption2.bold())
                    .padding(.horizontal, 6)
                    .padding(.vertical, 4)
                    .background(.ultraThinMaterial)
                    .clipShape(RoundedRectangle(cornerRadius: 5))
                    .position(x: snap.point.x + 44, y: snap.point.y - 18)
            }
            .allowsHitTesting(false)
        }
    }
}
