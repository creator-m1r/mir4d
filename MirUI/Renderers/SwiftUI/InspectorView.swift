
import SwiftUI

@MainActor
final class InspectorViewModel: ObservableObject {
    @Published var text = ""
    @Published var x = "0"
    @Published var y = "0"
    @Published var width = "100"
    @Published var height = "40"
    @Published var background = "#FFFFFFFF"
    @Published var visible = true
    @Published var enabled = true
    @Published var selectedType = ""

    func load(id: Int64?) {
        guard let id else {
            text = ""; x = "0"; y = "0"; width = "100"; height = "40"
            background = "#FFFFFFFF"; visible = true; enabled = true; selectedType = ""
            return
        }

        selectedType = get(id, "type") ?? ""
        text = get(id, "text") ?? ""
        x = get(id, "x") ?? "0"
        y = get(id, "y") ?? "0"
        width = get(id, "width") ?? "100"
        height = get(id, "height") ?? "40"
        background = get(id, "background") ?? "#FFFFFFFF"
        visible = get(id, "visible") == "false" ? false : true
        enabled = get(id, "enabled") == "false" ? false : true
    }

    func applyString(_ id: Int64, _ name: String, _ value: String) {
        value.withCString { valuePtr in
            MirUI_SetPropertyString(id, name.cString(using: .utf8), valuePtr)
        }
        MirUI_RenderFrame()
    }

    func applyDouble(_ id: Int64, _ name: String, _ value: String) {
        guard let number = Double(value) else { return }
        MirUI_SetPropertyDouble(id, name.cString(using: .utf8), number)
        MirUI_RenderFrame()
    }

    func applyBool(_ id: Int64, _ name: String, _ value: Bool) {
        MirUI_SetPropertyBool(id, name.cString(using: .utf8), value)
        MirUI_RenderFrame()
    }

    private func get(_ id: Int64, _ name: String) -> String? {
        let pointer = MirUI_GetPropertyString(id, name.cString(using: .utf8))
        return cStrToString(pointer)
    }
}

struct InspectorView: View {
    @StateObject private var vm = InspectorViewModel()
    @Binding var selectedWidgetId: Int64?

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Инспектор свойств")
                .font(.headline)

            if let id = selectedWidgetId {
                ScrollView {
                    VStack(alignment: .leading, spacing: 12) {
                        propertyText("Тип", vm.selectedType)

                        section("Текст") {
                            TextField("Текст", text: $vm.text, onCommit: {
                                vm.applyString(id, "text", vm.text)
                            })
                            .textFieldStyle(.roundedBorder)
                        }

                        section("Позиция") {
                            HStack {
                                numericField("X", value: $vm.x) { vm.applyDouble(id, "x", vm.x) }
                                numericField("Y", value: $vm.y) { vm.applyDouble(id, "y", vm.y) }
                            }
                        }

                        section("Размер") {
                            HStack {
                                numericField("Ш", value: $vm.width) { vm.applyDouble(id, "width", vm.width) }
                                numericField("В", value: $vm.height) { vm.applyDouble(id, "height", vm.height) }
                            }
                        }

                        section("Состояние") {
                            Toggle("Видимость", isOn: Binding(
                                get: { vm.visible },
                                set: {
                                    vm.visible = $0
                                    vm.applyBool(id, "visible", $0)
                                }
                            ))
                            Toggle("Доступность", isOn: Binding(
                                get: { vm.enabled },
                                set: {
                                    vm.enabled = $0
                                    vm.applyBool(id, "enabled", $0)
                                }
                            ))
                        }

                        section("Цвет фона") {
                            TextField("#FFFFFFFF", text: $vm.background, onCommit: {
                                vm.applyString(id, "background", vm.background)
                            })
                            .textFieldStyle(.roundedBorder)

                            Color(hex: vm.background)
                                .frame(width: 32, height: 20)
                                .clipShape(RoundedRectangle(cornerRadius: 4))
                        }

                        FontEditorView(
                            widgetId: id,
                            fontString: vm.getFont(id)
                        )
                    }
                    .padding(.vertical, 4)
                }
            } else {
                Text("Ничего не выделено")
                    .foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity, alignment: .center)
                    .padding(.vertical, 20)
            }

            Spacer()
        }
        .padding()
        .frame(width: 300)
        .background(Color.gray.opacity(0.05))
        .onAppear { vm.load(id: selectedWidgetId) }
        .onChange(of: selectedWidgetId) {  _, newValue in
            vm.load(id: newValue)
        }
    }

    private func section<Content: View>(_ title: String, @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 6) {
            Text(title)
                .font(.caption)
                .foregroundStyle(.secondary)
            content()
        }
    }

    private func numericField(_ label: String, value: Binding<String>, onCommit: @escaping () -> Void) -> some View {
        VStack(spacing: 2) {
            Text(label).font(.caption2).foregroundStyle(.secondary)
            TextField("0", text: value, onCommit: onCommit)
                .textFieldStyle(.roundedBorder)
                .frame(width: 70)
                .multilineTextAlignment(.center)
        }
    }

    private func propertyText(_ title: String, _ value: String) -> some View {
        HStack {
            Text(title).foregroundStyle(.secondary)
            Spacer()
            Text(value.isEmpty ? "—" : value)
        }
        .font(.caption)
    }
}

private extension InspectorViewModel {
    func getFont(_ id: Int64) -> String? {
        let pointer = MirUI_GetPropertyString(id, "font".cString(using: .utf8))
        return cStrToString(pointer)
    }
}
