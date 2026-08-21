// MirUI/Renderers/SwiftUI/FontEditorView.swift
// 🔤 Редактор шрифта для инспектора свойств.

import SwiftUI

class FontEditorViewModel: ObservableObject {
    @Published var family: String = "System"
    @Published var size: Double = 14
    @Published var weight: Int = 400
    @Published var style: Int = 0
    var widgetId: Int64 = 0

    let families = ["System", "Menlo", "Georgia", "Helvetica", "Times New Roman", "Courier"]
    let weights: [(Int, String)] = [
        (400, "Обычный"),
        (500, "Средний"),
        (600, "Полужирный"),
        (700, "Жирный"),
        (900, "Очень жирный")
    ]
    let styles: [(Int, String)] = [
        (0, "Обычный"),
        (1, "Курсив")
    ]

    func load(from fontString: String?) {
        guard let raw = fontString, !raw.isEmpty else {
            resetToDefault()
            return
        }
        let parts = raw.components(separatedBy: ";")
        if parts.count >= 1 { family = parts[0] }
        if parts.count >= 2 { size = Double(parts[1]) ?? 14 }
        if parts.count >= 3 { weight = Int(parts[2]) ?? 400 }
        if parts.count >= 4 { style = Int(parts[3]) ?? 0 }
    }

    func resetToDefault() {
        family = "System"
        size = 14
        weight = 400
        style = 0
    }

    func apply() {
        let fontString = "\(family);\(size);\(weight);\(style)"
        fontString.withCString { cStr in
            MirUI_SetPropertyString(widgetId, "font", cStr)
        }
        MirUI_RenderFrame()
    }
}

struct FontEditorView: View {
    @StateObject private var vm = FontEditorViewModel()

    let widgetId: Int64
    let initialFontString: String?

    init(widgetId: Int64, fontString: String?) {
        self.widgetId = widgetId
        self.initialFontString = fontString
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Шрифт")
                .font(.caption)
                .foregroundColor(.secondary)

            HStack {
                Text("Семейство:").font(.caption)
                Picker("", selection: $vm.family) {
                    ForEach(vm.families, id: \.self) { family in
                        Text(family).tag(family)
                    }
                }
                .pickerStyle(MenuPickerStyle())
                .frame(maxWidth: .infinity, alignment: .leading)
            }

            HStack {
                Text("Размер:").font(.caption)
                TextField("14", value: $vm.size, format: .number)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                    .frame(width: 60)
                Stepper("", value: $vm.size, in: 8...72)
                    .labelsHidden()
            }

            HStack {
                Text("Начертание:").font(.caption)
                Picker("", selection: $vm.weight) {
                    ForEach(vm.weights, id: \.0) { value, name in
                        Text(name).tag(value)
                    }
                }
                .pickerStyle(MenuPickerStyle())
                .frame(maxWidth: .infinity, alignment: .leading)
            }

            HStack {
                Text("Стиль:").font(.caption)
                Picker("", selection: $vm.style) {
                    ForEach(vm.styles, id: \.0) { value, name in
                        Text(name).tag(value)
                    }
                }
                .pickerStyle(MenuPickerStyle())
                .frame(maxWidth: .infinity, alignment: .leading)
            }

            Button("Применить шрифт") { vm.apply() }
                .font(.caption)
        }
        .onAppear {
            vm.widgetId = widgetId
            vm.load(from: initialFontString)
        }
    }
}
