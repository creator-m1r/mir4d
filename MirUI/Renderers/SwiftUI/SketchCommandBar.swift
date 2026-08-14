import SwiftUI

struct SketchCommandBar: View {
    let onToolSelected: (String) -> Void

    private let geometryTools: [(String, String, String)] = [
        ("select", "cursorarrow", "Выбор"),
        ("point", "circle", "Точка"),
        ("line", "line.diagonal", "Линия"),
        ("arc", "circle.dashed", "Дуга"),
        ("circle", "circle", "Окружность"),
        ("rectangle", "rectangle", "Прямоугольник")
    ]

    private let constraintTools: [(String, String, String)] = [
        ("coincident", "link", "Совпадение"),
        ("horizontal", "arrow.left.and.right", "Горизонталь"),
        ("vertical", "arrow.up.and.down", "Вертикаль"),
        ("parallel", "equal", "Параллельность"),
        ("perpendicular", "plus", "Перпендикулярность"),
        ("tangent", "arrow.up.right", "Касательность"),
        ("fix", "pin", "Фиксация")
    ]

    var body: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 6) {
                commandGroup(title: "Геометрия", items: geometryTools)
                Divider().frame(height: 28)
                commandGroup(title: "Ограничения", items: constraintTools)
                Divider().frame(height: 28)
                commandButton("dimension", "ruler", "Размер")
                commandButton("solve", "function", "Решить")
                commandButton("close", "checkmark.circle", "Завершить")
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 6)
        }
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 10))
    }

    @ViewBuilder
    private func commandGroup(title: String, items: [(String, String, String)]) -> some View {
        HStack(spacing: 4) {
            Text(title)
                .font(.system(size: 9, weight: .semibold))
                .foregroundStyle(.secondary)
                .padding(.horizontal, 4)

            ForEach(items, id: \.0) { item in
                commandButton(item.0, item.1, item.2)
            }
        }
    }

    private func commandButton(_ id: String, _ icon: String, _ title: String) -> some View {
        Button {
            onToolSelected(id)
        } label: {
            Label(title, systemImage: icon)
                .font(.system(size: 10, weight: .medium))
                .padding(.horizontal, 8)
                .padding(.vertical, 6)
        }
        .buttonStyle(.borderless)
        .background(Color.primary.opacity(0.06))
        .clipShape(RoundedRectangle(cornerRadius: 6))
    }
}
