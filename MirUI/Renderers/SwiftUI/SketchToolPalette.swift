import SwiftUI

struct SketchToolPalette: View {
    @Binding var activeTool: SketchTool

    private let columns = [GridItem(.adaptive(minimum: 52), spacing: 6)]

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("ЭСКИЗ")
                .font(.caption.bold())
                .foregroundStyle(.secondary)

            LazyVGrid(columns: columns, spacing: 6) {
                tool(.select, "cursorarrow", "Выбор")
                tool(.line, "line.diagonal", "Линия")
                tool(.arc, "circle.dashed", "Дуга")
                tool(.circle, "circle", "Окружность")
                tool(.rectangle, "rectangle", "Прямоугольник")
                tool(.trim, "scissors", "Обрезать")
                tool(.offset, "square.on.square", "Смещение")
                tool(.dimension, "ruler", "Размер")
                tool(.mirror, "arrow.left.and.right", "Зеркало")
                tool(.pattern, "square.grid.3x3", "Массив")
                tool(.spline, "waveform.path", "Сплайн")
            }
        }
        .padding(10)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 12))
    }

    private func tool(
        _ tool: SketchTool,
        _ symbol: String,
        _ title: String
    ) -> some View {
        Button {
            activeTool = tool
        } label: {
            VStack(spacing: 4) {
                Image(systemName: symbol)
                    .font(.system(size: 17))
                Text(title)
                    .font(.system(size: 9))
                    .lineLimit(1)
            }
            .frame(maxWidth: .infinity, minHeight: 44)
        }
        .buttonStyle(.bordered)
        .tint(activeTool == tool ? .cyan : .secondary)
    }
}
