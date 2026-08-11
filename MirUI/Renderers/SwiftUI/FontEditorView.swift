// MirUI/Renderers/SwiftUI/FontEditorView.swift
// 🔤 Редактор шрифта для инспектора свойств.
//
// Шрифт в MirUI хранится как строка специального формата:
//   "family;size;weight;style"
// Например: "System;14;400;0"
//   family — название шрифта (System, Menlo, Georgia…)
//   size   — размер в пунктах (14.0)
//   weight — жирность (400 = Regular, 500 = Medium, 700 = Bold)
//   style  — стиль (0 = Normal, 1 = Italic)
//
// FontEditorView разбирает эту строку на части, показывает их
// в удобных полях ввода и выпадающих списках, а при изменении
// собирает обратно и отправляет в C++ через мост.
//
// Как это выглядит в инспекторе:
//   ┌─────────────────────────────┐
//   │ Шрифт                       │
//   │ Семейство: [System    ▼]   │
//   │ Размер:    [14]  [ + – ]   │
//   │ Начертание:[Обычный  ▼]   │
//   │ Стиль:     [Обычный  ▼]   │
//   └─────────────────────────────┘
//
// Чистый SwiftUI, использует C-функции из MirUICppBridge.

import SwiftUI

// MARK: - ViewModel редактора шрифта

class FontEditorViewModel: ObservableObject {
    // Разобранные составляющие шрифта
    @Published var family: String = "System"
    @Published var size: Double = 14
    @Published var weight: Int = 400
    @Published var style: Int = 0
    
    // Ссылка на C++ мост (устанавливается извне)
    var widgetId: Int64 = 0
    
    // Список доступных семейств
    let families = ["System", "Menlo", "Georgia", "Helvetica", "Times New Roman", "Courier"]
    
    // Список начертаний (вес + имя)
    let weights: [(Int, String)] = [
        (400, "Обычный"),
        (500, "Средний"),
        (600, "Полужирный"),
        (700, "Жирный"),
        (900, "Очень жирный")
    ]
    
    // Список стилей
    let styles: [(Int, String)] = [
        (0, "Обычный"),
        (1, "Курсив")
    ]
    
    // ── Загрузить шрифт из строки ────────────────────────────
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
    
    // ── Сериализовать обратно в строку и применить ──────────
    func apply() {
        let fontString = "\(family);\(size);\(weight);\(style)"
        fontString.withCString { cStr in
            MirUI_SetPropertyString(widgetId, "font", cStr)
        }
        MirUI_RenderFrame()
    }
}

// MARK: - Само View редактора

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
            
            // Выбор семейства
            HStack {
                Text("Семейство:")
                    .font(.caption)
                Picker("", selection: $vm.family) {
                    ForEach(vm.families, id: \.self) { family in
                        Text(family).tag(family)
                    }
                }
                .pickerStyle(MenuPickerStyle())
                .frame(maxWidth: .infinity, alignment: .leading)
            }
            
            // Размер
            HStack {
                Text("Размер:")
                    .font(.caption)
                TextField("14", value: $vm.size, format: .number)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                    .frame(width: 60)
                Stepper("", value: $vm.size, in: 8...72)
                    .labelsHidden()
            }
            
            // Начертание
            HStack {
                Text("Начертание:")
                    .font(.caption)
                Picker("", selection: $vm.weight) {
                    ForEach(vm.weights, id: \.0) { (value, name) in
                        Text(name).tag(value)
                    }
                }
                .pickerStyle(MenuPickerStyle())
                .frame(maxWidth: .infinity, alignment: .leading)
            }
            
            // Стиль
            HStack {
                Text("Стиль:")
                    .font(.caption)
                Picker("", selection: $vm.style) {
                    ForEach(vm.styles, id: \.0) { (value, name) in
                        Text(name).tag(value)
                    }
                }
                .pickerStyle(MenuPickerStyle())
                .frame(maxWidth: .infinity, alignment: .leading)
            }
            
            // Кнопка применения (можно применять сразу по изменению)
            Button("Применить шрифт") { vm.apply() }
                .font(.caption)
        }
        .onAppear {
            vm.widgetId = widgetId
            vm.load(from: initialFontString)
        }
    }
}

// MARK: - C-функции (сигнатуры)

@_silgen_name("MirUI_SetPropertyString")
func MirUI_SetPropertyString(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ value: UnsafePointer<CChar>?)

@_silgen_name("MirUI_RenderFrame")
func MirUI_RenderFrame()