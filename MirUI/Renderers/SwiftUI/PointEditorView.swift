// MirUI/Renderers/SwiftUI/PointEditorView.swift
// 📍 Редактор координат точки (Point) для инспектора свойств.
//
// Позволяет редактировать позицию виджета через два поля ввода: X и Y.
// При изменении значений вызывает MirUI_ResizeWidget (которая на самом деле
// меняет и позицию, и размер) или специализированную функцию MirUI_SetPosition,
// если она будет добавлена в мост.
//
// Поддерживает групповое редактирование: если передано несколько widgetIds,
// изменения применяются ко всем выделенным виджетам.
//
// Чистый SwiftUI, использует C-функции из MirUICppBridge.

import SwiftUI

// MARK: - ViewModel редактора координат

class PointEditorViewModel: ObservableObject {
    @Published var x: String = "0"
    @Published var y: String = "0"
    
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
            x = "0"; y = "0"
            return
        }
        x = getStringProperty(firstId, "x") ?? "0"
        y = getStringProperty(firstId, "y") ?? "0"
    }
    
    // Применить позицию ко всем выделенным виджетам
    func applyPosition() {
        guard !widgetIds.isEmpty,
              let newX = Double(x),
              let newY = Double(y) else { return }
        
        for id in widgetIds {
            // Получаем текущие размеры виджета
            let widthStr = getStringProperty(id, "width") ?? "100"
            let heightStr = getStringProperty(id, "height") ?? "40"
            let w = Double(widthStr) ?? 100
            let h = Double(heightStr) ?? 40
            
            // Вызываем ResizeWidget с новыми координатами и старыми размерами
            MirUI_ResizeWidget(id, w, h, newX, newY)
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

struct PointEditorView: View {
    @StateObject private var vm = PointEditorViewModel()
    
    init(widgetIds: [Int64]) {
        _vm = StateObject(wrappedValue: PointEditorViewModel())
        DispatchQueue.main.async {
            vm.load(from: widgetIds)
        }
    }
    
    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Позиция")
                .font(.caption)
                .foregroundColor(.secondary)
            
            HStack(spacing: 16) {
                // X
                VStack(spacing: 2) {
                    Text("X").font(.caption2).foregroundColor(.secondary)
                    TextField("0", text: $vm.x, onCommit: { vm.applyPosition() })
                        .textFieldStyle(RoundedBorderTextFieldStyle())
                        .frame(width: 60)
                        .multilineTextAlignment(.center)
                }
                
                // Y
                VStack(spacing: 2) {
                    Text("Y").font(.caption2).foregroundColor(.secondary)
                    TextField("0", text: $vm.y, onCommit: { vm.applyPosition() })
                        .textFieldStyle(RoundedBorderTextFieldStyle())
                        .frame(width: 60)
                        .multilineTextAlignment(.center)
                }
            }
            
            Button("Применить позицию") { vm.applyPosition() }
                .font(.caption)
        }
    }
}

// MARK: - C-функции (сигнатуры)

@_silgen_name("MirUI_ResizeWidget")
func MirUI_ResizeWidget(_ widgetId: Int64, _ newWidth: Double, _ newHeight: Double, _ newX: Double, _ newY: Double)

@_silgen_name("MirUI_GetPropertyString")
func MirUI_GetPropertyString(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?

@_silgen_name("MirUI_RenderFrame")
func MirUI_RenderFrame()