import SwiftUI

struct MIR4DBrushOverlay: View {
    @ObservedObject private var indicator = MIR4DBrushIndicator.shared

    var body: some View {
        GeometryReader { geo in
            if indicator.active {
                let frac = CGPoint(
                    x: 0.5 + indicator.position.x * 0.5,
                    y: 0.5 - indicator.position.y * 0.5
                )
                let center = CGPoint(
                    x: frac.x * geo.size.width,
                    y: frac.y * geo.size.height
                )
                let base = min(geo.size.width, geo.size.height)
                let r = max(CGFloat(indicator.radius) * base * 0.5, 14)

                Circle()
                    .stroke(Color.white.opacity(0.85), lineWidth: 2)
                    .background(Circle().fill(Color.cyan.opacity(0.14)))
                    .frame(width: r * 2, height: r * 2)
                    .position(center)

                Circle()
                    .fill(Color.white)
                    .frame(width: 6, height: 6)
                    .position(center)

                if let mode = indicator.mode {
                    Text(mode.shortTitle)
                        .font(.system(size: 11, weight: .semibold))
                        .foregroundStyle(.white)
                        .padding(4)
                        .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 6))
                        .position(x: center.x, y: center.y - r - 16)
                }
            }
        }
        .allowsHitTesting(false)
    }
}
