// MirUI/Renderers/SwiftUI/PointEditorView.swift
// 📍 Редактор координат точки (Point) для инспектора свойств.
//
// Назначение:
// - отображение координат X/Y;
// - загрузка координат из MirUI;
// - редактирование координат;
// - применение позиции к одному или нескольким объектам;
// - безопасная работа со Swift 6 concurrency.
//
// Архитектурный принцип:
// PointEditorView отвечает только за интерфейс.
// PointEditorViewModel отвечает за состояние и взаимодействие с MirUI C API.

import SwiftUI

// MARK: - View Model

@MainActor
final class PointEditorViewModel: ObservableObject {

    // MARK: Published state

    @Published var x: String = "0"
    @Published var y: String = "0"

    @Published private(set) var isApplying = false
    @Published private(set) var lastError: String?
    @Published private(set) var hasLoaded = false

    // MARK: Selection

    private(set) var widgetIds: [Int64] = []

    // MARK: Initialization

    init(widgetIds: [Int64] = []) {
        self.widgetIds = widgetIds
    }

    // MARK: Loading

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

    // MARK: Apply

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

    // MARK: Reset

    func resetPosition() {

        x = "0"
        y = "0"

        applyPosition()
    }

    // MARK: Property access

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

// MARK: - Point Editor View

struct PointEditorView: View {

    // MARK: Input

    let widgetIds: [Int64]

    // MARK: State

    @StateObject private var vm: PointEditorViewModel

    // MARK: Initialization

    init(widgetIds: [Int64]) {

        self.widgetIds = widgetIds

        _vm = StateObject(
            wrappedValue:
                PointEditorViewModel(
                    widgetIds: widgetIds
                )
        )
    }

    // MARK: Body

    var body: some View {

        VStack(
            alignment: .leading,
            spacing: 10
        ) {

            // Header

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

            // Coordinates

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

            // Actions

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

            // Error

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

    // MARK: Coordinate field

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
