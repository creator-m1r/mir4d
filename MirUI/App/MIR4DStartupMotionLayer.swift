import SwiftUI

/// Lightweight SwiftUI-only motion layer for the launch sequence.
/// No renderer, Metal/OpenGL, or MirEngine work is performed here.
struct MIR4DStartupMotionLayer: View {
    var phase: Double = 0
    @State private var pulse = false
    @State private var scan = false
    @State private var rotation: Double = 0

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                Canvas { context, size in
                    let center = CGPoint(x: size.width * 0.5, y: size.height * 0.42)
                    let radius = min(size.width, size.height) * 0.28

                    for index in 0..<7 {
                        let r = radius * (0.38 + CGFloat(index) * 0.105)
                        var path = Path()
                        path.addEllipse(in: CGRect(x: center.x - r, y: center.y - r, width: r * 2, height: r * 2))
                        context.stroke(path, with: .color(.white.opacity(0.035)), lineWidth: 1)
                    }

                    var vertical = Path()
                    vertical.move(to: CGPoint(x: center.x, y: center.y - radius))
                    vertical.addLine(to: CGPoint(x: center.x, y: center.y + radius))
                    context.stroke(vertical, with: .color(.white.opacity(0.055)), lineWidth: 1)

                    var horizontal = Path()
                    horizontal.move(to: CGPoint(x: center.x - radius, y: center.y))
                    horizontal.addLine(to: CGPoint(x: center.x + radius, y: center.y))
                    context.stroke(horizontal, with: .color(.white.opacity(0.055)), lineWidth: 1)
                }
                .opacity(0.95)

                Circle()
                    .stroke(.white.opacity(pulse ? 0.16 : 0.06), lineWidth: 1)
                    .frame(width: 260, height: 260)
                    .scaleEffect(pulse ? 1.08 : 0.94)
                    .blur(radius: 0.2)

                Rectangle()
                    .fill(
                        LinearGradient(
                            colors: [.clear, .white.opacity(0.10), .clear],
                            startPoint: .leading,
                            endPoint: .trailing
                        )
                    )
                    .frame(height: 1)
                    .offset(y: scan ? proxy.size.height * 0.38 : -proxy.size.height * 0.38)
                    .opacity(0.55)

                Circle()
                    .fill(.white.opacity(0.75))
                    .frame(width: 3, height: 3)
                    .shadow(color: .white.opacity(0.8), radius: 7)
                    .offset(x: 128 * cos(rotation), y: 74 * sin(rotation))
            }
            .frame(width: proxy.size.width, height: proxy.size.height)
            .blendMode(.screen)
            .allowsHitTesting(false)
        }
        .onAppear {
            withAnimation(.easeInOut(duration: 2.8).repeatForever(autoreverses: true)) {
                pulse = true
            }
            withAnimation(.linear(duration: 4.8).repeatForever(autoreverses: false)) {
                scan = true
            }
            withAnimation(.linear(duration: 8).repeatForever(autoreverses: false)) {
                rotation = .pi * 2
            }
        }
    }
}
