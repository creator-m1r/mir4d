import SwiftUI

struct CreateBodyView: View {
    @ObservedObject var appState: CADAppState
    @Environment(\.dismiss) private var dismiss

    @State private var width = 100.0
    @State private var depth = 60.0
    @State private var height = 40.0

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            header
            dimensionField("Ширина", value: $width)
            dimensionField("Глубина", value: $depth)
            dimensionField("Высота", value: $height)

            HStack {
                Spacer()
                Button("Отмена") { dismiss() }
                    .keyboardShortcut(.cancelAction)
                Button("Создать тело") {
                    createBody()
                }
                .buttonStyle(.borderedProminent)
                .keyboardShortcut(.defaultAction)
            }
        }
        .padding(22)
        .frame(width: 360)
    }

    private var header: some View {
        VStack(alignment: .leading, spacing: 5) {
            Label("Новое тело", systemImage: "cube.transparent")
                .font(.system(size: 18, weight: .bold))
            Text("Прямоугольный профиль → выдавливание → сетка → 3D-сцена")
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
        }
    }

    private func dimensionField(_ title: String, value: Binding<Double>) -> some View {
        HStack {
            Text(title)
                .frame(width: 100, alignment: .leading)
            TextField("0", value: value, format: .number.precision(.fractionLength(2)))
                .textFieldStyle(.roundedBorder)
            Text("mm")
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .frame(width: 32, alignment: .leading)
        }
    }

    private func createBody() {
        guard MIR4DModelCommands.shared.createBox(
            appState: appState,
            width: width,
            depth: depth,
            height: height
        ) != nil else {
            return
        }

        dismiss()
    }
}
