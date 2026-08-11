// MirUI/Renderers/SwiftUI/ColorTokenPicker.swift
// 🎨 Выбор цвета для любого токена — удобный компонент с палитрой.
//
// ColorTokenPicker показывает текущий цвет в виде закрашенного кружка
// и при нажатии открывает системную палитру (ColorPicker).
// После выбора цвета он автоматически вызывает C-функцию моста
// для обновления цвета в теме или стиле виджета.
//
// Использование:
//   ColorTokenPicker(
//       tokenKey: "interface.accent",
//       label: "Акцент",
//       onColorChanged: { newColor in ... }  // дополнительный колбэк
//   )
//
// Где взять текущий цвет:
//   - Для токенов темы: MirUI_GetThemeColor(tokenKey)
//   - Для полей стиля виджета: MirUI_GetWidgetStyleField(widgetType, state, fieldName)
//
// Компонент можно использовать:
//   • В редакторе тем — для изменения любого цветового токена.
//   • В инспекторе виджетов — для изменения цвета фона/текста/рамки
//     выделенного виджета.
//
// Чистый SwiftUI, использует C-функции моста.

import SwiftUI

// MARK: - ViewModel для пикера цвета

class ColorTokenPickerViewModel: ObservableObject {
    @Published var selectedColor: Color = .blue
    @Published var isPickerShown = false
    
    // Какую C-функцию вызывать для установки цвета
    enum ApplyMode {
        case themeToken(String)          // токен темы (например, "interface.accent")
        case widgetStyleField(widgetType: String, widgetState: String, fieldName: String)
        case custom(((Color) -> Void)?)  // произвольный колбэк
    }
    
    var applyMode: ApplyMode = .themeToken("")
    
    // Загрузить начальный цвет из C++ темы/стиля
    func loadColor() {
        var hex: String = "#000000FF"
        
        switch applyMode {
        case .themeToken(let token):
            if let cStr = MirUI_GetThemeColor(token.cString(using: .utf8)) {
                hex = String(cString: cStr)
            }
        case .widgetStyleField(let widgetType, let widgetState, let fieldName):
            if let cStr = MirUI_GetWidgetStyleField(
                widgetType.cString(using: .utf8),
                widgetState.cString(using: .utf8),
                fieldName.cString(using: .utf8)
            ) {
                hex = String(cString: cStr)
            }
        case .custom:
            // Цвет задаётся извне
            return
        }
        
        selectedColor = colorFromHex(hex)
    }
    
    // Применить выбранный цвет через мост
    func applyColor() {
        let hex = colorToHex(selectedColor)
        
        switch applyMode {
        case .themeToken(let token):
            MirUI_SetThemeColor(token.cString(using: .utf8), hex.cString(using: .utf8))
            
        case .widgetStyleField(let widgetType, let widgetState, let fieldName):
            MirUI_SetWidgetStyleField(
                widgetType.cString(using: .utf8),
                widgetState.cString(using: .utf8),
                fieldName.cString(using: .utf8),
                hex.cString(using: .utf8)
            )
            
        case .custom(let callback):
            callback?(selectedColor)
        }
        
        MirUI_RenderFrame()
    }
    
    // Преобразование Color <-> HEX
    func colorFromHex(_ hex: String) -> Color {
        guard hex.count == 9, hex.hasPrefix("#") else { return .blue }
        let r = Double(Int(hex.dropFirst().prefix(2), radix: 16) ?? 0) / 255
        let g = Double(Int(hex.dropFirst(3).prefix(2), radix: 16) ?? 0) / 255
        let b = Double(Int(hex.dropFirst(5).prefix(2), radix: 16) ?? 0) / 255
        return Color(red: r, green: g, blue: b)
    }
    
    func colorToHex(_ color: Color) -> String {
        let nsColor = NSColor(color)
        guard let rgbColor = nsColor.usingColorSpace(.sRGB) else { return "#000000FF" }
        let r = Int(round(rgbColor.redComponent * 255))
        let g = Int(round(rgbColor.greenComponent * 255))
        let b = Int(round(rgbColor.blueComponent * 255))
        let a = Int(round(rgbColor.alphaComponent * 255))
        return String(format: "#%02X%02X%02X%02X", r, g, b, a)
    }
}

// MARK: - Представление пикера цвета

struct ColorTokenPicker: View {
    let label: String
    @ObservedObject var vm: ColorTokenPickerViewModel
    
    init(label: String, applyMode: ColorTokenPickerViewModel.ApplyMode) {
        self.label = label
        _vm = ObservedObject(wrappedValue: ColorTokenPickerViewModel())
        vm.applyMode = applyMode
        vm.loadColor()
    }
    
    // Дополнительный инициализатор для произвольного колбэка
    init(label: String, color: Color, onColorChanged: @escaping (Color) -> Void) {
        self.label = label
        _vm = ObservedObject(wrappedValue: ColorTokenPickerViewModel())
        vm.selectedColor = color
        vm.applyMode = .custom(onColorChanged)
    }
    
    var body: some View {
        HStack {
            Text(label + ":")
                .frame(width: 100, alignment: .leading)
            
            // Закрашенный кружок
            Circle()
                .fill(vm.selectedColor)
                .frame(width: 24, height: 24)
                .overlay(Circle().stroke(Color.gray.opacity(0.5), lineWidth: 1))
                .onTapGesture {
                    vm.isPickerShown.toggle()
                }
                .popover(isPresented: $vm.isPickerShown) {
                    VStack(spacing: 12) {
                        ColorPicker("Выберите цвет", selection: $vm.selectedColor)
                            .padding()
                        
                        HStack {
                            Button("Отмена") {
                                vm.isPickerShown = false
                            }
                            Spacer()
                            Button("Применить") {
                                vm.applyColor()
                                vm.isPickerShown = false
                            }
                            .buttonStyle(.borderedProminent)
                        }
                        .padding(.horizontal)
                    }
                    .frame(width: 250)
                }
            
            // Текстовое поле с HEX-значением (для продвинутых пользователей)
            TextField("#HEX", text: Binding(
                get: { vm.colorToHex(vm.selectedColor) },
                set: { newHex in
                    let color = vm.colorFromHex(newHex)
                    vm.selectedColor = color
                    vm.applyColor()
                }
            ))
            .font(.caption)
            .frame(width: 100)
            
            Button("✓") {
                vm.applyColor()
            }
            .font(.caption)
        }
        .onAppear {
            vm.loadColor()
        }
    }
}

// MARK: - C-функции (сигнатуры)

@_silgen_name("MirUI_GetThemeColor")
func MirUI_GetThemeColor(_ colorToken: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?

@_silgen_name("MirUI_SetThemeColor")
func MirUI_SetThemeColor(_ colorToken: UnsafePointer<CChar>?, _ hexColor: UnsafePointer<CChar>?)

@_silgen_name("MirUI_GetWidgetStyleField")
func MirUI_GetWidgetStyleField(_ widgetType: UnsafePointer<CChar>?, _ widgetState: UnsafePointer<CChar>?, _ fieldName: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?

@_silgen_name("MirUI_SetWidgetStyleField")
func MirUI_SetWidgetStyleField(_ widgetType: UnsafePointer<CChar>?, _ widgetState: UnsafePointer<CChar>?, _ fieldName: UnsafePointer<CChar>?, _ value: UnsafePointer<CChar>?)

@_silgen_name("MirUI_RenderFrame")
func MirUI_RenderFrame()