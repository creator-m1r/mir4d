// MirUI/Renderers/SwiftUI/EnumEditorView.swift
// 📋 Редактор перечисления (Enum) для инспектора свойств.
//
// Некоторые свойства могут принимать только одно значение из заранее
// заданного списка — например, выравнивание текста: Left, Center, Right.
// EnumEditorView отображает выпадающий список (Picker) с этими вариантами
// и при выборе отправляет новое значение в C++ через мост.
//
// Компонент полностью переиспользуемый:
//   EnumEditorView(
//       widgetId: id,
//       propertyName: "alignment",
//       currentValue: "Left",
//       possibleValues: ["Left", "Center", "Right"]
//   )
//
// При смене значения автоматически вызываются:
//   MirUI_SetPropertyString(widgetId, propertyName, newValue)
//   MirUI_RenderFrame()
//
// Таким образом, изменение сразу попадает в историю Undo/Redo,
// и холст с инспектором обновляются.
//
// Чистый SwiftUI, использует C-функции из MirUICppBridge.

import SwiftUI

struct EnumEditorView: View {
    let widgetId: Int64
    let propertyName: String
    let possibleValues: [String]
    
    @State private var selectedValue: String
    
    init(widgetId: Int64, propertyName: String, currentValue: String, possibleValues: [String]) {
        self.widgetId = widgetId
        self.propertyName = propertyName
        self.possibleValues = possibleValues
        _selectedValue = State(initialValue: currentValue)
    }
    
    var body: some View {
        HStack {
            Text(propertyName.capitalized + ":")
                .font(.caption)
                .foregroundColor(.secondary)
            Picker("", selection: $selectedValue) {
                ForEach(possibleValues, id: \.self) { value in
                    Text(value).tag(value)
                }
            }
            .pickerStyle(MenuPickerStyle())
            .frame(maxWidth: .infinity, alignment: .leading)
            .onChange(of: selectedValue) { newValue in
                // Отправляем новое значение в C++
                newValue.withCString { cStr in
                    MirUI_SetPropertyString(widgetId, propertyName.cString(using: .utf8)!, cStr)
                }
                MirUI_RenderFrame()
            }
        }
    }
}

// MARK: - C-функции (сигнатуры)

@_silgen_name("MirUI_SetPropertyString")
func MirUI_SetPropertyString(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ value: UnsafePointer<CChar>?)

@_silgen_name("MirUI_RenderFrame")
func MirUI_RenderFrame()