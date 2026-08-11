// MirUI/Renderers/SwiftUI/InsetsEditorView.swift
// 📏 Редактор отступов (Insets) для инспектора свойств.
//
// Отступы определяют расстояние от границы виджета до его содержимого
// или до соседних элементов. InsetsEditorView позволяет редактировать
// четыре поля одновременно: Top (верх), Right (право), Bottom (низ), Left (лево).
// Каждое поле связано с C++ свойством через мост: при изменении значения
// вызывается MirUI_SetPropertyDouble с соответствующим ключом
// ("paddingTop", "paddingRight", "paddingBottom", "paddingLeft").
//
// Редактор поддерживает групповое редактирование:
// если в selectedWidgetIds передан массив из нескольких ID,
// изменения применяются ко всем выделенным виджетам.
//
// Чистый SwiftUI, использует C-функции из MirUICppBridge.

import SwiftUI

// MARK: - ViewModel редактора отступов

class InsetsEditorViewModel: ObservableObject {
    @Published var top: String = "0"
    @Published var right: String = "0"
    @Published var bottom: String = "0"
    @Published var left: String = "0"
    
    // ID виджетов, к которым применяются изменения (может быть несколько)
    var widgetIds: [Int64] = []
    
    // Загрузить значения из одного виджета
    func load(from widgetId: Int64) {
        widgetIds = [widgetId]
        loadFromFirst()
    }
    
    // Загрузить значения из нескольких виджетов (берём значения первого)
    func load(from ids: [Int64]) {
        widgetIds = ids
        loadFromFirst()
    }
    
    private func loadFromFirst() {
        guard let firstId = widgetIds.first else {
            top = "0"; right = "0"; bottom = "0"; left = "0"
            return
        }
        top = getStringProperty(firstId, "paddingTop") ?? "0"
        right = getStringProperty(firstId, "paddingRight") ?? "0"
        bottom = getStringProperty(firstId, "paddingBottom") ?? "0"
        left = getStringProperty(firstId, "paddingLeft") ?? "0"
    }
    
    // Применить все четыре значения ко всем выделенным виджетам
    func applyAll() {
        guard !widgetIds.isEmpty,
              let t = Double(top),
              let r = Double(right),
              let b = Double(bottom),
              let l = Double(left) else { return }
        
        for id in widgetIds {
            MirUI_SetPropertyDouble(id, "paddingTop", t)
            MirUI_SetPropertyDouble(id, "paddingRight", r)
            MirUI_SetPropertyDouble(id, "paddingBottom", b)
            MirUI_SetPropertyDouble(id, "paddingLeft", l)
        }
        MirUI_RenderFrame()
    }
    
    private func getStringProperty(_ id: Int64, _ name: String) -> String? {
        let cStr = MirUI_GetPropertyString(id, name.cString(using: .utf8)!)
        guard let swiftStr = cStr.flatMap({ String(cString: $0 }) else { return nil }
        return swiftStr.isEmpty ? nil : swiftStr
    }
}

// MARK: - Представление редактора

struct InsetsEditorView: View {
    @StateObject private var vm = InsetsEditorViewModel()
    
    // Конфигурация: один виджет или несколько
    init(widgetIds: [Int64]) {
        _vm = StateObject(wrappedValue: InsetsEditorViewModel())
        // Загружаем данные при создании
        DispatchQueue.main.async {
            vm.load(from: widgetIds)
        }
    }
    
    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Отступы (Padding)")
                .font(.caption)
                .foregroundColor(.secondary)
            
            // Четыре поля в виде матрицы 2x2 с подписями
            HStack(spacing: 16) {
                // Верх
                VStack(spacing: 2) {
                    Text("Top").font(.caption2).foregroundColor(.secondary)
                    TextField("0", text: $vm.top, onCommit: { vm.applyAll() })
                        .textFieldStyle(RoundedBorderTextFieldStyle())
                        .frame(width: 60)
                        .multilineTextAlignment(.center)
                }
                
                // Право
                VStack(spacing: 2) {
                    Text("Right").font(.caption2).foregroundColor(.secondary)
                    TextField("0", text: $vm.right, onCommit: { vm.applyAll() })
                        .textFieldStyle(RoundedBorderTextFieldStyle())
                        .frame(width: 60)
                        .multilineTextAlignment(.center)
                }
            }
            HStack(spacing: 16) {
                // Низ
                VStack(spacing: 2) {
                    Text("Bottom").font(.caption2).foregroundColor(.secondary)
                    TextField("0", text: $vm.bottom, onCommit: { vm.applyAll() })
                        .textFieldStyle(RoundedBorderTextFieldStyle())
                        .frame(width: 60)
                        .multilineTextAlignment(.center)
                }
                
                // Лево
                VStack(spacing: 2) {
                    Text("Left").font(.caption2).foregroundColor(.secondary)
                    TextField("0", text: $vm.left, onCommit: { vm.applyAll() })
                        .textFieldStyle(RoundedBorderTextFieldStyle())
                        .frame(width: 60)
                        .multilineTextAlignment(.center)
                }
            }
            
            // Кнопка применения
            Button("Применить отступы") { vm.applyAll() }
                .font(.caption)
        }
    }
}

// MARK: - C-функции (сигнатуры)

@_silgen_name("MirUI_SetPropertyDouble")
func MirUI_SetPropertyDouble(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ value: Double)

@_silgen_name("MirUI_GetPropertyString")
func MirUI_GetPropertyString(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?

@_silgen_name("MirUI_RenderFrame")
func MirUI_RenderFrame()