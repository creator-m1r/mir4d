import SwiftUI

struct SketchPropertiesPanel: View {
    @ObservedObject var session: SketchSessionModel
    let line: SketchLineParametersUI?
    let onChange: (SketchLineParameterUI, Double) -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.md) {
            HStack(spacing: 8) {
                Image(systemName: "slider.horizontal.3")
                    .foregroundStyle(MirTheme.Colors.accentBright)
                VStack(alignment: .leading, spacing: 2) {
                    Text("Свойства")
                        .font(MirTheme.Typography.bodySemibold)
                        .foregroundStyle(MirTheme.Colors.textPrimary)
                    Text(line.map { "Линия #\($0.geometryID)" } ?? "Выберите геометрию")
                        .font(MirTheme.Typography.status)
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                }
                Spacer()
            }

            if let line {
                parameterGroup(title: "Геометрия") {
                    parameterField("Начало X", value: line.startX, parameter: .startX)
                    parameterField("Начало Y", value: line.startY, parameter: .startY)
                    parameterField("Конец X", value: line.endX, parameter: .endX)
                    parameterField("Конец Y", value: line.endY, parameter: .endY)
                    parameterField("Длина", value: line.length, parameter: .length)
                    parameterField("Угол", value: line.angleDegrees, parameter: .angleDegrees)
                }

                parameterGroup(title: "Состояние") {
                    statusRow("Ограничение", line.horizontal ? "Горизонтальная" : line.vertical ? "Вертикальная" : "Свободное направление", line.horizontal || line.vertical)
                    statusRow("Степени свободы", session.degreesOfFreedom.map(String.init) ?? "—", false)
                    statusRow("Решатель", session.solverStatus, false)
                }
            } else {
                emptyState
            }
        }
        .padding(MirTheme.Spacing.lg)
        .frame(minWidth: 260, alignment: .leading)
        .background(MirTheme.Colors.panel)
        .overlay(alignment: .leading) {
            Rectangle().fill(MirTheme.Colors.border).frame(width: 1)
        }
    }

    private var emptyState: some View {
        VStack(spacing: 8) {
            Image(systemName: "cursorarrow.click.2")
                .font(.system(size: 20, weight: .light))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text("Выберите геометрию")
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
            Text("Параметры выбранного элемента появятся здесь")
                .font(MirTheme.Typography.status)
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .multilineTextAlignment(.center)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 28)
    }

    private func parameterGroup<Content: View>(title: String, @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 8) {
            Text(title.uppercased())
                .font(MirTheme.Typography.section)
                .tracking(0.4)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            VStack(spacing: 6) { content() }
                .padding(10)
                .background(MirTheme.Colors.surface)
                .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
                .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.medium).stroke(MirTheme.Colors.border, lineWidth: 1))
        }
    }

    private func statusRow(_ title: String, _ value: String, _ positive: Bool) -> some View {
        HStack(spacing: 8) {
            Text(title).font(MirTheme.Typography.caption).foregroundStyle(MirTheme.Colors.textSecondary)
            Spacer()
            if positive { Image(systemName: "checkmark.circle.fill").foregroundStyle(MirTheme.Colors.success) }
            Text(value).font(MirTheme.Typography.status).foregroundStyle(positive ? MirTheme.Colors.success : MirTheme.Colors.textPrimary)
        }
        .padding(.vertical, 3)
    }

    @ViewBuilder
    private func parameterField(_ title: String, value: Double, parameter: SketchLineParameterUI) -> some View {
        HStack(spacing: 8) {
            Text(title).font(MirTheme.Typography.caption).foregroundStyle(MirTheme.Colors.textSecondary)
                .frame(maxWidth: .infinity, alignment: .leading)
            TextField("", value: Binding(get: { value }, set: { onChange(parameter, $0) }), format: .number.precision(.fractionLength(3)))
                .textFieldStyle(.plain)
                .font(MirTheme.Typography.numeric)
                .foregroundStyle(MirTheme.Colors.textPrimary)
                .multilineTextAlignment(.trailing)
                .padding(.horizontal, 8).padding(.vertical, 5)
                .frame(width: 105)
                .background(MirTheme.Colors.surfaceRaised)
                .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
                .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border, lineWidth: 1))
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
