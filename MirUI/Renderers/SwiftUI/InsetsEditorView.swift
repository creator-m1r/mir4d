
import SwiftUI

@MainActor
final class InsetsEditorViewModel: ObservableObject {

    @Published var top: String = "0"
    @Published var right: String = "0"
    @Published var bottom: String = "0"
    @Published var left: String = "0"

    private(set) var widgetIds: [Int64] = []

    func load(from widgetId: Int64) {
        load(from: [widgetId])
    }

    func load(from ids: [Int64]) {
        widgetIds = ids
        loadFromFirst()
    }

    private func loadFromFirst() {
        guard let firstId = widgetIds.first else {
            resetValues()
            return
        }

        top = getStringProperty(firstId, "paddingTop") ?? "0"
        right = getStringProperty(firstId, "paddingRight") ?? "0"
        bottom = getStringProperty(firstId, "paddingBottom") ?? "0"
        left = getStringProperty(firstId, "paddingLeft") ?? "0"
    }

    func applyAll() {
        guard !widgetIds.isEmpty else {
            return
        }

        guard
            let topValue = Double(top),
            let rightValue = Double(right),
            let bottomValue = Double(bottom),
            let leftValue = Double(left)
        else {
            return
        }

        for id in widgetIds {
            MirUI_SetPropertyDouble(
                id,
                "paddingTop",
                topValue
            )

            MirUI_SetPropertyDouble(
                id,
                "paddingRight",
                rightValue
            )

            MirUI_SetPropertyDouble(
                id,
                "paddingBottom",
                bottomValue
            )

            MirUI_SetPropertyDouble(
                id,
                "paddingLeft",
                leftValue
            )
        }

        MirUI_RenderFrame()
    }

    func resetValues() {
        top = "0"
        right = "0"
        bottom = "0"
        left = "0"
    }

    func resetAndApply() {
        resetValues()
        applyAll()
    }

    private func getStringProperty(
        _ id: Int64,
        _ name: String
    ) -> String? {

        let nameCString = name.cString(using: .utf8)

        guard let nameCString else {
            return nil
        }

        let cStr = MirUI_GetPropertyString(
            id,
            nameCString
        )

        guard let cStr else {
            return nil
        }

        let value = mirCString(cStr) ?? ""

        return value.isEmpty ? nil : value
    }
}

struct InsetsEditorView: View {

    let widgetIds: [Int64]

    @StateObject private var vm = InsetsEditorViewModel()

    init(widgetIds: [Int64]) {
        self.widgetIds = widgetIds
    }

    var body: some View {
        VStack(
            alignment: .leading,
            spacing: 10
        ) {

            HStack {

                VStack(
                    alignment: .leading,
                    spacing: 2
                ) {
                    Text("Отступы")
                        .font(.caption)
                        .fontWeight(.semibold)

                    Text("Padding")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }

                Spacer()

                if widgetIds.count > 1 {
                    Text("\(widgetIds.count) объекта")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }

            HStack {

                Spacer()

                insetField(
                    title: "Top",
                    value: $vm.top
                )

                Spacer()
            }

            HStack(
                spacing: 16
            ) {

                insetField(
                    title: "Left",
                    value: $vm.left
                )

                insetField(
                    title: "Right",
                    value: $vm.right
                )
            }

            HStack {

                Spacer()

                insetField(
                    title: "Bottom",
                    value: $vm.bottom
                )

                Spacer()
            }

            Divider()

            HStack(
                spacing: 8
            ) {

                Button {
                    vm.resetAndApply()
                } label: {
                    Label(
                        "Сбросить",
                        systemImage: "arrow.counterclockwise"
                    )
                }
                .buttonStyle(.bordered)
                .controlSize(.small)

                Spacer()

                Button {
                    vm.applyAll()
                } label: {
                    Label(
                        "Применить",
                        systemImage: "checkmark"
                    )
                }
                .buttonStyle(.borderedProminent)
                .controlSize(.small)
            }
        }
        .padding(8)

        .task(id: widgetIds) {
            vm.load(from: widgetIds)
        }
    }

    @ViewBuilder
    private func insetField(
        title: String,
        value: Binding<String>
    ) -> some View {

        VStack(
            spacing: 3
        ) {

            Text(title)
                .font(.caption2)
                .foregroundStyle(.secondary)

            TextField(
                "0",
                text: value
            )
            .textFieldStyle(.roundedBorder)
            .frame(width: 64)
            .multilineTextAlignment(.center)
            .onSubmit {
                vm.applyAll()
            }
        }
    }
}
