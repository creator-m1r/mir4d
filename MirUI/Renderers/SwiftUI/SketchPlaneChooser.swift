import SwiftUI

struct SketchPlaneChooser: View {
    var onPick: (SketchPlaneAnchor) -> Void

    private let columns = [
        GridItem(.fixed(190), spacing: 14),
        GridItem(.fixed(190), spacing: 14),
        GridItem(.fixed(190), spacing: 14)
    ]

    var body: some View {
        ZStack {
            Color.black.opacity(0.55)
                .ignoresSafeArea()
                .allowsHitTesting(true)

            VStack(spacing: 18) {
                VStack(spacing: 6) {
                    Text("ВЫБОР ПЛОСКОСТИ ПОСТРОЕНИЯ ЭСКИЗА")
                        .font(.headline)
                        .foregroundStyle(.white)
                    Text("Камера плавно встанет перпендикулярно выбранной плоскости. Плоскость останется прозрачной — видны проекции 3D-геометрии для привязки.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                        .multilineTextAlignment(.center)
                        .frame(maxWidth: 560)
                }

                LazyVGrid(columns: columns, spacing: 14) {
                    planeCard(.xy, "xy", "plane.fill")
                    planeCard(.yz, "yz", "rectangle.split.2x2")
                    planeCard(.zx, "zx", "square.split.diagonal")
                }

                Text("Совет: позже плоскость можно сменить через панель работы с эскизом.")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            .padding(26)
            .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 18))
            .overlay(RoundedRectangle(cornerRadius: 18).stroke(.quaternary, lineWidth: 1))
            .frame(maxWidth: 640)
        }
    }

    private func planeCard(_ anchor: SketchPlaneAnchor, _ tag: String, _ symbol: String) -> some View {
        Button {
            onPick(anchor)
        } label: {
            VStack(spacing: 8) {
                Image(systemName: symbol)
                    .font(.system(size: 30))
                    .foregroundStyle(.cyan)
                Text("Плоскость \(anchor.name)")
                    .font(.system(size: 15, weight: .semibold))
                    .foregroundStyle(.white)
                Text("Вид: \(anchor.preset.titleRU)")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            .frame(maxWidth: .infinity, minHeight: 110)
            .contentShape(Rectangle())
        }
        .buttonStyle(.borderedProminent)
        .tint(.accentColor.opacity(0.18))
    }
}
