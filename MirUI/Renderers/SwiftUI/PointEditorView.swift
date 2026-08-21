
import SwiftUI

@MainActor
final class PointEditorViewModel: ObservableObject {

    @Published var x: String = "0"
    @Published var y: String = "0"

    @Published private(set) var isApplying = false
    @Published private(set) var lastError: String?
    @Published private(set) var hasLoaded = false

    private(set) var widgetIds: [Int64] = []

    init(widgetIds: [Int64] = []) {
        self.widgetIds = widgetIds
    }

    func load(from widgetId: Int64) {
        widgetIds = [widgetId]
        loadFromFirst()
    }

    func load(from ids: [Int64]) {
        widgetIds = ids
        loadFromFirst()
    }

    func reload() {
        loadFromFirst()
    }

    private func loadFromFirst() {

        hasLoaded = false
        lastError = nil

        guard let firstId = widgetIds.first else {
            x = "0"
            y = "0"
            hasLoaded = true
            return
        }

        x = getStringProperty(
            firstId,
            "x"
        ) ?? "0"

        y = getStringProperty(
            firstId,
            "y"
        ) ?? "0"

        hasLoaded = true
    }

    func applyPosition() {

        guard !widgetIds.isEmpty else {
            lastError = "Нет выбранных объектов."
            return
        }

        guard let newX = Double(x.replacingOccurrences(of: ",", with: ".")) else {
            lastError = "Некорректное значение X."
            return
        }

        guard let newY = Double(y.replacingOccurrences(of: ",", with: ".")) else {
            lastError = "Некорректное значение Y."
            return
        }

        isApplying = true
        lastError = nil

        for id in widgetIds {

            let widthString =
                getStringProperty(
                    id,
                    "width"
                ) ?? "100"

            let heightString =
                getStringProperty(
                    id,
                    "height"
                ) ?? "40"

            let width =
                Double(
                    widthString.replacingOccurrences(
                        of: ",",
                        with: "."
                    )
                ) ?? 100

            let height =
                Double(
                    heightString.replacingOccurrences(
                        of: ",",
                        with: "."
                    )
                ) ?? 40

            MirUI_ResizeWidget(
                id,
                width,
                height,
                newX,
                newY
            )
        }

        MirUI_RenderFrame()

        isApplying = false
    }

    func resetPosition() {

        x = "0"
        y = "0"

        applyPosition()
    }

    private func getStringProperty(
        _ id: Int64,
        _ name: String
    ) -> String? {

        guard let cName = name.cString(using: .utf8) else {
            return nil
        }

        guard let cString =
            MirUI_GetPropertyString(
                id,
                cName
            )
        else {
            return nil
        }

        let value =
            String(
                cString: cString
            )

        return value.isEmpty ? nil : value
    }
}

struct PointEditorView: View {

    let widgetIds: [Int64]

    @StateObject private var vm: PointEditorViewModel

    init(widgetIds: [Int64]) {

        self.widgetIds = widgetIds

        _vm = StateObject(
            wrappedValue:
                PointEditorViewModel(
                    widgetIds: widgetIds
                )
        )
    }

    var body: some View {

        VStack(
            alignment: .leading,
            spacing: 10
        ) {

            HStack {

                Text("Позиция")
                    .font(.caption)
                    .foregroundStyle(.secondary)

                Spacer()

                if widgetIds.count > 1 {

                    Text(
                        "\(widgetIds.count) объекта"
                    )
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                }
            }

            HStack(
                spacing: 12
            ) {

                coordinateField(
                    title: "X",
                    text: $vm.x
                )

                coordinateField(
                    title: "Y",
                    text: $vm.y
                )
            }

            HStack(
                spacing: 8
            ) {

                Button {
                    vm.applyPosition()
                } label: {

                    if vm.isApplying {

                        ProgressView()
                            .controlSize(.small)

                    } else {

                        Label(
                            "Применить",
                            systemImage: "checkmark"
                        )
                    }
                }
                .buttonStyle(.borderedProminent)
                .disabled(
                    vm.isApplying
                    || widgetIds.isEmpty
                )

                Button {
                    vm.resetPosition()
                } label: {

                    Label(
                        "Сбросить",
                        systemImage: "arrow.counterclockwise"
                    )
                }
                .buttonStyle(.bordered)
                .disabled(vm.isApplying)
            }

            if let error = vm.lastError {

                Label(
                    error,
                    systemImage: "exclamationmark.triangle"
                )
                .font(.caption2)
                .foregroundStyle(.red)
            }
        }
        .padding(10)
        .task(id: widgetIds) {

            vm.load(
                from: widgetIds
            )
        }
    }

    @ViewBuilder
    private func coordinateField(
        title: String,
        text: Binding<String>
    ) -> some View {

        VStack(
            spacing: 3
        ) {

            Text(title)
                .font(.caption2)
                .foregroundStyle(.secondary)

            TextField(
                "0",
                text: text
            )
            .textFieldStyle(.roundedBorder)
            .frame(width: 70)
            .multilineTextAlignment(.center)
            .onSubmit {

                vm.applyPosition()
            }
        }
    }
}
