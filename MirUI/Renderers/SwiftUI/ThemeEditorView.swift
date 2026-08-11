// MirUI/Renderers/SwiftUI/ThemeEditorView.swift
// 🎨 Панель визуального редактора тем — позволяет менять тему в реальном времени.
//
// Теперь пользователь может:
//   • Переключаться между светлой и тёмной темой.
//   • Менять основные цвета (фон, поверхность, текст, акцент, граница).
//   • Настраивать метрики (отступ, высоту тулбара, радиус скругления).
//   • Изменять шрифты (семейство, размер, жирность).
//   • Регулировать длительность анимаций.
//
// Все изменения мгновенно применяются к C++ теме через мост,
// холст и инспектор перерисовываются автоматически.
//
// Чистый SwiftUI, использует C-функции моста.

import SwiftUI

// MARK: - ViewModel редактора тем

class ThemeEditorViewModel: ObservableObject {
    // Цвета
    @Published var backgroundColor: Color = .white
    @Published var surfaceColor: Color = .white
    @Published var textPrimaryColor: Color = .black
    @Published var accentColor: Color = .blue
    @Published var borderColor: Color = .gray
    
    // Метрики
    @Published var spacingM: Double = 12.0
    @Published var toolbarHeight: Double = 44.0
    @Published var radiusM: Double = 8.0
    
    // Шрифты
    @Published var fontFamily: String = "System"
    @Published var fontSize: Double = 14.0
    @Published var fontWeight: Int = 400
    
    // Анимации
    @Published var animationDuration: Double = 0.25
    
    // Текущая тема
    @Published var currentThemeName: String = "Светлая тема"
    
    // Загрузить текущие значения из C++ темы
    func loadFromTheme() {
        // Цвета
        backgroundColor = colorFromHex(String(cString: MirUI_GetThemeColor("interface.background")))
        surfaceColor = colorFromHex(String(cString: MirUI_GetThemeColor("interface.surface")))
        textPrimaryColor = colorFromHex(String(cString: MirUI_GetThemeColor("interface.textPrimary")))
        accentColor = colorFromHex(String(cString: MirUI_GetThemeColor("interface.accent")))
        borderColor = colorFromHex(String(cString: MirUI_GetThemeColor("interface.border")))
        
        // Метрики
        spacingM = MirUI_GetThemeMetric("spacing.m")
        toolbarHeight = MirUI_GetThemeMetric("toolbar.height")
        radiusM = MirUI_GetThemeMetric("radius.m")
        
        // Шрифт
        let fontStr = String(cString: MirUI_GetThemeFont("typography.body"))
        let parts = fontStr.components(separatedBy: ";")
        if parts.count >= 3 {
            fontFamily = parts[0]
            fontSize = Double(parts[1]) ?? 14.0
            fontWeight = Int(parts[2]) ?? 400
        }
        
        // Анимации
        animationDuration = MirUI_GetThemeAnimationDuration()
        
        // Имя темы
        currentThemeName = String(cString: MirUI_CurrentThemeName())
    }
    
    // Применить изменения
    func applyColor(_ color: Color, for token: String) {
        let hex = colorToHex(color)
        MirUI_SetThemeColor(token.cString(using: .utf8), hex.cString(using: .utf8))
    }
    
    func applyMetric(_ value: Double, for token: String) {
        MirUI_SetThemeMetric(token.cString(using: .utf8), value)
    }
    
    func applyFont() {
        let fontStr = "\(fontFamily);\(fontSize);\(fontWeight);0"
        MirUI_SetThemeFont("typography.body", fontStr.cString(using: .utf8))
    }
    
    func applyAnimationDuration() {
        MirUI_SetThemeAnimationDuration(animationDuration)
    }
    
    func switchTheme(_ id: String) {
        MirUI_SwitchTheme(id.cString(using: .utf8))
        MirUI_RenderFrame()
        loadFromTheme()
    }
    
    // Преобразование Color <-> HEX
    func colorFromHex(_ hex: String) -> Color {
        guard hex.count == 9, hex.hasPrefix("#") else { return .black }
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

// MARK: - Панель редактора тем

struct ThemeEditorView: View {
    @StateObject private var vm = ThemeEditorViewModel()
    @State private var selectedTab = 0
    
    var body: some View {
        VStack(spacing: 0) {
            // Заголовок
            HStack {
                Text("Редактор темы")
                    .font(.headline)
                Spacer()
                Text(vm.currentThemeName)
                    .font(.caption)
                    .foregroundColor(.secondary)
            }
            .padding()
            
            // Выбор темы
            HStack {
                Button("Светлая") { vm.switchTheme("mir.light") }
                    .buttonStyle(.borderedProminent)
                    .tint(vm.currentThemeName.contains("Светлая") ? .blue : .gray)
                Button("Тёмная") { vm.switchTheme("mir.dark") }
                    .buttonStyle(.borderedProminent)
                    .tint(vm.currentThemeName.contains("Тёмная") ? .blue : .gray)
            }
            .padding(.bottom)
            
            // Вкладки
            Picker("", selection: $selectedTab) {
                Text("Цвета").tag(0)
                Text("Метрики").tag(1)
                Text("Шрифт").tag(2)
                Text("Анимации").tag(3)
            }
            .pickerStyle(.segmented)
            .padding(.horizontal)
            
            Divider()
            
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    switch selectedTab {
                    case 0: colorsTab
                    case 1: metricsTab
                    case 2: fontsTab
                    case 3: animationsTab
                    default: EmptyView()
                    }
                }
                .padding()
            }
            
            // Кнопка «Обновить из темы»
            Button("Обновить из темы") {
                vm.loadFromTheme()
            }
            .padding(.bottom)
        }
        .frame(width: 300)
        .background(Color.gray.opacity(0.05))
        .onAppear {
            vm.loadFromTheme()
        }
    }
    
    // MARK: - Вкладка «Цвета»
    
    var colorsTab: some View {
        VStack(alignment: .leading, spacing: 12) {
            ColorPickerRow("Фон", color: $vm.backgroundColor) {
                vm.applyColor(vm.backgroundColor, for: "interface.background")
            }
            ColorPickerRow("Поверхность", color: $vm.surfaceColor) {
                vm.applyColor(vm.surfaceColor, for: "interface.surface")
            }
            ColorPickerRow("Текст", color: $vm.textPrimaryColor) {
                vm.applyColor(vm.textPrimaryColor, for: "interface.textPrimary")
            }
            ColorPickerRow("Акцент", color: $vm.accentColor) {
                vm.applyColor(vm.accentColor, for: "interface.accent")
            }
            ColorPickerRow("Граница", color: $vm.borderColor) {
                vm.applyColor(vm.borderColor, for: "interface.border")
            }
        }
    }
    
    // MARK: - Вкладка «Метрики»
    
    var metricsTab: some View {
        VStack(alignment: .leading, spacing: 12) {
            MetricRow("Отступ (M)", value: $vm.spacingM) {
                vm.applyMetric(vm.spacingM, for: "spacing.m")
            }
            MetricRow("Высота тулбара", value: $vm.toolbarHeight) {
                vm.applyMetric(vm.toolbarHeight, for: "toolbar.height")
            }
            MetricRow("Радиус (M)", value: $vm.radiusM) {
                vm.applyMetric(vm.radiusM, for: "radius.m")
            }
        }
    }
    
    // MARK: - Вкладка «Шрифт»
    
    var fontsTab: some View {
        VStack(alignment: .leading, spacing: 12) {
            // Семейство
            HStack {
                Text("Семейство:")
                TextField("System", text: $vm.fontFamily, onCommit: { vm.applyFont() })
                    .textFieldStyle(.roundedBorder)
            }
            // Размер
            HStack {
                Text("Размер:")
                Slider(value: $vm.fontSize, in: 8...72, step: 1)
                Text("\(Int(vm.fontSize))pt")
                    .frame(width: 40)
                Button("✓") { vm.applyFont() }
            }
            // Жирность
            HStack {
                Text("Жирность:")
                Picker("", selection: $vm.fontWeight) {
                    Text("Обычный").tag(400)
                    Text("Средний").tag(500)
                    Text("Полужирный").tag(600)
                    Text("Жирный").tag(700)
                }
                .pickerStyle(.segmented)
                .onChange(of: vm.fontWeight) { _ in vm.applyFont() }
            }
        }
    }
    
    // MARK: - Вкладка «Анимации»
    
    var animationsTab: some View {
        VStack(alignment: .leading, spacing: 12) {
            HStack {
                Text("Длительность (сек):")
                Slider(value: $vm.animationDuration, in: 0.1...2.0, step: 0.05)
                Text(String(format: "%.2f", vm.animationDuration))
                    .frame(width: 40)
            }
            .onChange(of: vm.animationDuration) { _ in
                vm.applyAnimationDuration()
            }
        }
    }
}

// MARK: - Вспомогательные View

struct ColorPickerRow: View {
    let title: String
    @Binding var color: Color
    let onApply: () -> Void
    
    init(_ title: String, color: Binding<Color>, onApply: @escaping () -> Void) {
        self.title = title
        self._color = color
        self.onApply = onApply
    }
    
    var body: some View {
        HStack {
            Text(title + ":")
                .frame(width: 100, alignment: .leading)
            ColorPicker("", selection: $color)
                .onChange(of: color) { _ in onApply() }
            Button("✓") { onApply() }
                .font(.caption)
        }
    }
}

struct MetricRow: View {
    let title: String
    @Binding var value: Double
    let onApply: () -> Void
    
    init(_ title: String, value: Binding<Double>, onApply: @escaping () -> Void) {
        self.title = title
        self._value = value
        self.onApply = onApply
    }
    
    var body: some View {
        HStack {
            Text(title + ":")
                .frame(width: 120, alignment: .leading)
            Slider(value: $value, in: 0...100, step: 1)
            Text("\(Int(value))px")
                .frame(width: 40)
            Button("✓") { onApply() }
                .font(.caption)
        }
    }
}

// MARK: - C-функции (сигнатуры)

@_silgen_name("MirUI_GetThemeColor")
func MirUI_GetThemeColor(_ colorToken: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?

@_silgen_name("MirUI_SetThemeColor")
func MirUI_SetThemeColor(_ colorToken: UnsafePointer<CChar>?, _ hexColor: UnsafePointer<CChar>?)

@_silgen_name("MirUI_GetThemeMetric")
func MirUI_GetThemeMetric(_ metricToken: UnsafePointer<CChar>?) -> Double

@_silgen_name("MirUI_SetThemeMetric")
func MirUI_SetThemeMetric(_ metricToken: UnsafePointer<CChar>?, _ value: Double)

@_silgen_name("MirUI_GetThemeFont")
func MirUI_GetThemeFont(_ fontToken: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?

@_silgen_name("MirUI_SetThemeFont")
func MirUI_SetThemeFont(_ fontToken: UnsafePointer<CChar>?, _ fontString: UnsafePointer<CChar>?)

@_silgen_name("MirUI_GetThemeAnimationDuration")
func MirUI_GetThemeAnimationDuration() -> Double

@_silgen_name("MirUI_SetThemeAnimationDuration")
func MirUI_SetThemeAnimationDuration(_ duration: Double)

@_silgen_name("MirUI_SwitchTheme")
func MirUI_SwitchTheme(_ themeId: UnsafePointer<CChar>?)

@_silgen_name("MirUI_CurrentThemeName")
func MirUI_CurrentThemeName() -> UnsafePointer<CChar>?

@_silgen_name("MirUI_RenderFrame")
func MirUI_RenderFrame()