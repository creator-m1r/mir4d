import SwiftUI

struct SketchPropertiesPanel: View {
    @ObservedObject var session: SketchSessionModel
    let line: SketchLineParametersUI?
    let onChange: (SketchLineParameterUI, Double) -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Свойства")
                .font(.headline)

            if let line {
                Text("Линия #\(line.geometryID)")
                    .font(.subheadline.weight(.semibold))

                parameterField("Начало X", value: line.startX, parameter: .startX)
                parameterField("Начало Y", value: line.startY, parameter: .startY)
                parameterField("Конец X", value: line.endX, parameter: .endX)
                parameterField("Конец Y", value: line.endY, parameter: .endY)
                parameterField("Длина", value: line.length, parameter: .length)
                parameterField("Угол", value: line.angleDegrees, parameter: .angleDegrees)

                Divider()

                Text(line.horizontal ? "✓ Горизонтальная" : line.vertical ? "✓ Вертикальная" : "Свободное направление")
                    .font(.caption)
                    .foregroundStyle(.secondary)

                Text("DOF: \(session.degreesOfFreedom.map(String.init) ?? "—")")
                    .font(.caption)
                    .foregroundStyle(.secondary)

                Text("Solver: \(session.solverStatus)")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            } else {
                Text("Выберите геометрию")
                    .foregroundStyle(.secondary)
            }
        }
        .padding(16)
        .frame(minWidth: 240, alignment: .leading)
    }

    @ViewBuilder
    private func parameterField(
        _ title: String,
        value: Double,
        parameter: SketchLineParameterUI
    ) -> some View {
        HStack {
            Text(title)
                .frame(maxWidth: .infinity, alignment: .leading)

            TextField("", value: Binding(
                get: { value },
                set: { onChange(parameter, $0) }
            ), format: .number.precision(.fractionLength(3)))
            .textFieldStyle(.roundedBorder)
            .frame(width: 105)
        }
    }
}

struct SketchLineParametersUI: Equatable {
    let geometryID: UInt32
    let startX: Double
    let startY: Double
    let endX: Double
    let endY: Double
    let length: Double
    let angleDegrees: Double
    let horizontal: Bool
    let vertical: Bool
}

enum SketchLineParameterUI {
    case startX, startY, endX, endY, length, angleDegrees
}
