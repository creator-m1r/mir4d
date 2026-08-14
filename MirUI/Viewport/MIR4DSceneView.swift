import SwiftUI

/// Lightweight 4D scene preview for the macOS interface.
///
/// This view intentionally depends only on the UI time value. The C++ engine
/// bridge will replace the local transform calculation in a later step.
struct MIR4DSceneView: View {
    let time: Double

    private var normalizedTime: Double {
        min(max(time / 10.0, 0.0), 1.0)
    }

    private var xOffset: CGFloat {
        CGFloat(-180.0 + normalizedTime * 360.0)
    }

    private var rotation: Angle {
        .degrees(normalizedTime * 360.0)
    }

    var body: some View {
        GeometryReader { geometry in
            ZStack {
                sceneGrid(in: geometry.size)

                SceneObject()
                    .rotation3DEffect(
                        .degrees(18),
                        axis: (x: 1, y: 0, z: 0)
                    )
                    .rotationEffect(rotation)
                    .offset(x: xOffset)
                    .animation(.linear(duration: 0.05), value: time)

                VStack {
                    HStack {
                        Label("4D SCENE", systemImage: "cube.transparent")
                            .font(.system(size: 10, weight: .bold))
                            .foregroundStyle(.secondary)
                        Spacer()
                        Text(String(format: "T = %.2f s", time))
                            .font(.system(size: 11, weight: .semibold, design: .monospaced))
                            .foregroundStyle(.cyan)
                    }
                    .padding(14)

                    Spacer()

                    HStack {
                        Text("X")
                        Spacer()
                        Text(String(format: "%+.1f", normalizedTime * 200.0 - 100.0))
                    }
                    .font(.system(size: 10, design: .monospaced))
                    .foregroundStyle(.secondary)
                    .padding(.horizontal, 14)
                    .padding(.bottom, 12)
                }
            }
        }
        .clipped()
        .background(Color(red: 0.025, green: 0.035, blue: 0.05))
    }

    private func sceneGrid(in size: CGSize) -> some View {
        Canvas { context, canvasSize in
            let spacing: CGFloat = 32
            var path = Path()

            var x: CGFloat = 0
            while x <= canvasSize.width {
                path.move(to: CGPoint(x: x, y: 0))
                path.addLine(to: CGPoint(x: x, y: canvasSize.height))
                x += spacing
            }

            var y: CGFloat = 0
            while y <= canvasSize.height {
                path.move(to: CGPoint(x: 0, y: y))
                path.addLine(to: CGPoint(x: canvasSize.width, y: y))
                y += spacing
            }

            context.stroke(
                path,
                with: .color(.white.opacity(0.035)),
                lineWidth: 0.5
            )
        }
        .frame(width: size.width, height: size.height)
        .allowsHitTesting(false)
    }
}

private struct SceneObject: View {
    var body: some View {
        ZStack {
            RoundedRectangle(cornerRadius: 12)
                .stroke(.cyan.opacity(0.8), lineWidth: 2)
                .frame(width: 180, height: 120)

            RoundedRectangle(cornerRadius: 12)
                .stroke(.blue.opacity(0.45), lineWidth: 1)
                .frame(width: 140, height: 90)

            Image(systemName: "cube.transparent")
                .font(.system(size: 46, weight: .thin))
                .foregroundStyle(.cyan)
        }
        .shadow(color: .cyan.opacity(0.25), radius: 18)
    }
}
