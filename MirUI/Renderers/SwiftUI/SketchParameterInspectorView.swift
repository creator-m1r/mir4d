import SwiftUI

struct SketchParameterInspectorView: View {
    @ObservedObject var state: SketchInspectorState
    @ObservedObject var dimensions: SketchDimensionState

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            Text("Параметры")
                .font(.headline)

            Text(state.geometry.type)
                .font(.subheadline.weight(.semibold))

            if let x1 = state.geometry.x1 { valueRow("X1", x1) }
            if let y1 = state.geometry.y1 { valueRow("Y1", y1) }
            if let x2 = state.geometry.x2 { valueRow("X2", x2) }
            if let y2 = state.geometry.y2 { valueRow("Y2", y2) }
            if let length = state.geometry.length { valueRow("Длина", length) }
            if let radius = state.geometry.radius { valueRow("Радиус", radius) }
            if let angle = state.geometry.angle { valueRow("Угол", angle, unit: "°") }

            Divider()

            HStack {
                Text("Размеров")
                Spacer()
                Text("\(dimensions.dimensions.count)")
                    .monospacedDigit()
            }
            .font(.caption)
        }
        .padding(12)
        .frame(maxWidth: .infinity, alignment: .leading)
    }

    private func valueRow(_ title: String, _ value: Double, unit: String = "мм") -> some View {
        HStack {
            Text(title)
            Spacer()
            Text(String(format: "%.3f %@", value, unit))
                .font(.system(.caption, design: .monospaced))
        }
    }
}
