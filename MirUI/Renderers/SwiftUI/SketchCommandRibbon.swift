import SwiftUI

struct SketchCommandRibbon: View {
    @ObservedObject var appState: CADAppState

    var body: some View {
        HStack(spacing: 0) {
            group("ПОСТРОЕНИЕ") {
                item("Точка", "circle") { appState.selectedTool = "point" }
                item("Линия", "line.diagonal") { appState.selectedTool = "line" }
                item("Прямоугольник", "rectangle") { appState.selectedTool = "rectangle" }
                item("Окружность", "circle.dashed") { appState.selectedTool = "circle" }
                item("Дуга", "arc") { appState.selectedTool = "arc" }
            }

            divider

            group("ОГРАНИЧЕНИЯ") {
                item("Горизонталь", "h.square") { notify("Горизонтальное ограничение") }
                item("Вертикаль", "v.square") { notify("Вертикальное ограничение") }
                item("Совпадение", "circle.fill") { notify("Ограничение совпадения") }
                item("Фиксация", "lock") { notify("Фиксация геометрии") }
            }

            divider

            group("РАЗМЕРЫ") {
                item("Размер", "ruler") { appState.selectedTool = "dimension" }
                item("Угол", "angle") { notify("Угловой размер") }
            }

            divider

            group("ОПЕРАЦИИ") {
                item("Обрезать", "scissors") { notify("Обрезка геометрии") }
                item("Смещение", "square.on.square") { notify("Смещение геометрии") }
                item("Зеркало", "arrow.left.and.right") { notify("Зеркальное отражение") }
            }

            Spacer(minLength: 8)

            Button {
                appState.selectWorkbench(.model)
            } label: {
                Label("Завершить эскиз", systemImage: "checkmark.circle.fill")
                    .font(.system(size: 11, weight: .semibold))
                    .padding(.horizontal, 10)
                    .padding(.vertical, 7)
            }
            .buttonStyle(.borderedProminent)
            .tint(.cyan)
        }
        .padding(.horizontal, 12)
        .frame(minHeight: 58)
        .background(.ultraThinMaterial)
    }

    private func group<Content: View>(_ title: String, @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(title)
                .font(.system(size: 8, weight: .bold))
                .foregroundStyle(.secondary)
            HStack(spacing: 4) {
                content()
            }
        }
        .padding(.horizontal, 5)
    }

    private func item(_ title: String, _ icon: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            VStack(spacing: 3) {
                Image(systemName: icon)
                    .font(.system(size: 14))
                Text(title)
                    .font(.system(size: 8))
                    .lineLimit(1)
            }
            .frame(minWidth: 50, minHeight: 36)
        }
        .buttonStyle(.plain)
        .foregroundStyle(.primary)
        .background(Color.white.opacity(0.035))
        .clipShape(RoundedRectangle(cornerRadius: 6))
    }

    private var divider: some View {
        Divider()
            .frame(height: 34)
            .padding(.horizontal, 4)
    }

    private func notify(_ text: String) {
        appState.showNotification("Sketch: \(text)", type: .info)
    }
}
