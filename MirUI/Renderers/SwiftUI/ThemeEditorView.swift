// MirUI/Renderers/SwiftUI/ThemeEditorView.swift
// 🎨 Редактор темы MIR 4D.

import SwiftUI
#if os(macOS)
import AppKit
#endif

@MainActor
final class ThemeEditorViewModel: ObservableObject {
    @Published var backgroundColor: Color = .white
    @Published var surfaceColor: Color = .white
    @Published var textPrimaryColor: Color = .black
    @Published var accentColor: Color = .blue
    @Published var borderColor: Color = .gray

    @Published var spacingM: Double = 12
    @Published var toolbarHeight: Double = 44
    @Published var radiusM: Double = 8

    @Published var fontFamily: String = "System"
    @Published var fontSize: Double = 14
    @Published var fontWeight: Int = 400

    @Published var animationDuration: Double = 0.25
    @Published var currentThemeName: String = "Светлая тема"

    func loadFromTheme() {
        backgroundColor = Color(hex: cStrToString(MirUI_GetThemeColor("interface.background")) ?? "#000000FF")
        surfaceColor = Color(hex: cStrToString(MirUI_GetThemeColor("interface.surface")) ?? "#000000FF")
        textPrimaryColor = Color(hex: cStrToString(MirUI_GetThemeColor("interface.textPrimary")) ?? "#FFFFFFFF")
        accentColor = Color(hex: cStrToString(MirUI_GetThemeColor("interface.accent")) ?? "#4A90E2FF")
        borderColor = Color(hex: cStrToString(MirUI_GetThemeColor("interface.border")) ?? "#808080FF")

        spacingM = MirUI_GetThemeMetric("spacing.m")
        toolbarHeight = MirUI_GetThemeMetric("toolbar.height")
        radiusM = MirUI_GetThemeMetric("radius.m")

        let fontStr = cStrToString(MirUI_GetThemeFont("typography.body")) ?? "System;14;400;0"
        let parts = fontStr.components(separatedBy: ";")
        fontFamily = parts.indices.contains(0) ? parts[0] : "System"
        fontSize = parts.indices.contains(1) ? (Double(parts[1]) ?? 14) : 14
        fontWeight = parts.indices.contains(2) ? (Int(parts[2]) ?? 400) : 400

        animationDuration = MirUI_GetThemeAnimationDuration()
        currentThemeName = cStrToString(MirUI_CurrentThemeName()) ?? currentThemeName
    }

    func applyColor(_ color: Color, for token: String) {
        let hex = colorToHex(color)
        token.withCString { tokenPtr in
            hex.withCString { hexPtr in
                MirUI_SetThemeColor(tokenPtr, hexPtr)
            }
        }
    }

    func applyMetric(_ value: Double, for token: String) {
        token.withCString { tokenPtr in
            MirUI_SetThemeMetric(tokenPtr, value)
        }
    }

    func applyFont() {
        let fontString = "\(fontFamily);\(fontSize);\(fontWeight);0"
        fontString.withCString { fontPtr in
            MirUI_SetThemeFont("typography.body", fontPtr)
        }
        MirUI_RenderFrame()
    }

    func applyAnimationDuration() {
        MirUI_SetThemeAnimationDuration(animationDuration)
        MirUI_RenderFrame()
    }

    func switchTheme(_ id: String) {
        id.withCString { idPtr in
            MirUI_SwitchTheme(idPtr)
        }
        MirUI_RenderFrame()
        loadFromTheme()
    }

    func colorToHex(_ color: Color) -> String {
#if os(macOS)
        let nsColor = NSColor(color)
        guard let rgb = nsColor.usingColorSpace(.sRGB) else { return "#000000FF" }
        let r = Int((rgb.redComponent * 255).rounded())
        let g = Int((rgb.greenComponent * 255).rounded())
        let b = Int((rgb.blueComponent * 255).rounded())
        let a = Int((rgb.alphaComponent * 255).rounded())
        return String(format: "#%02X%02X%02X%02X", r, g, b, a)
#else
        return "#000000FF"
#endif
    }
}

struct ThemeEditorView: View {
    @StateObject private var vm = ThemeEditorViewModel()
    @State private var selectedTab = 0

    var body: some View {
        VStack(spacing: 0) {
            HStack {
                Text("Редактор темы").font(.headline)
                Spacer()
                Text(vm.currentThemeName).font(.caption).foregroundColor(.secondary)
            }
            .padding()

            HStack {
                Button("Светлая") { vm.switchTheme("mir.light") }
                    .buttonStyle(.borderedProminent)
                    .tint(vm.currentThemeName.contains("Светлая") ? .blue : .gray)
                Button("Тёмная") { vm.switchTheme("mir.dark") }
                    .buttonStyle(.borderedProminent)
                    .tint(vm.currentThemeName.contains("Тёмная") ? .blue : .gray)
            }
            .padding(.bottom)

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

            Button("Обновить из темы") { vm.loadFromTheme() }
                .padding(.bottom)
        }
        .frame(width: 300)
        .background(Color.gray.opacity(0.05))
        .onAppear { vm.loadFromTheme() }
    }

    private var colorsTab: some View {
        VStack(alignment: .leading, spacing: 12) {
            ColorPickerRow("Фон", color: $vm.backgroundColor) { vm.applyColor(vm.backgroundColor, for: "interface.background") }
            ColorPickerRow("Поверхность", color: $vm.surfaceColor) { vm.applyColor(vm.surfaceColor, for: "interface.surface") }
            ColorPickerRow("Текст", color: $vm.textPrimaryColor) { vm.applyColor(vm.textPrimaryColor, for: "interface.textPrimary") }
            ColorPickerRow("Акцент", color: $vm.accentColor) { vm.applyColor(vm.accentColor, for: "interface.accent") }
            ColorPickerRow("Граница", color: $vm.borderColor) { vm.applyColor(vm.borderColor, for: "interface.border") }
        }
    }

    private var metricsTab: some View {
        VStack(alignment: .leading, spacing: 12) {
            MetricRow("Отступ (M)", value: $vm.spacingM) { vm.applyMetric(vm.spacingM, for: "spacing.m") }
            MetricRow("Высота тулбара", value: $vm.toolbarHeight) { vm.applyMetric(vm.toolbarHeight, for: "toolbar.height") }
            MetricRow("Радиус (M)", value: $vm.radiusM) { vm.applyMetric(vm.radiusM, for: "radius.m") }
        }
    }

    private var fontsTab: some View {
        VStack(alignment: .leading, spacing: 12) {
            HStack {
                Text("Семейство:")
                TextField("System", text: $vm.fontFamily, onCommit: vm.applyFont)
                    .textFieldStyle(.roundedBorder)
            }
            HStack {
                Text("Размер:")
                Slider(value: $vm.fontSize, in: 8...72, step: 1)
                Text("\(Int(vm.fontSize))pt").frame(width: 40)
                Button("✓") { vm.applyFont() }
            }
            HStack {
                Text("Жирность:")
                Picker("", selection: $vm.fontWeight) {
                    Text("Обычный").tag(400)
                    Text("Средний").tag(500)
                    Text("Полужирный").tag(600)
                    Text("Жирный").tag(700)
                }
                .pickerStyle(.segmented)
                .onChange(of: vm.fontWeight) { _, _ in vm.applyFont() }
            }
        }
    }

    private var animationsTab: some View {
        VStack(alignment: .leading, spacing: 12) {
            HStack {
                Text("Длительность (сек):")
                Slider(value: $vm.animationDuration, in: 0.1...2.0, step: 0.05)
                Text(String(format: "%.2f", vm.animationDuration)).frame(width: 40)
            }
            .onChange(of: vm.animationDuration) { _, _ in vm.applyAnimationDuration() }
        }
    }
}

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
            Text(title + ":").frame(width: 100, alignment: .leading)
            ColorPicker("", selection: $color)
                .onChange(of: color) { _, _ in onApply() }
            Button("✓") { onApply() }.font(.caption)
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
            Text(title + ":").frame(width: 120, alignment: .leading)
            Slider(value: $value, in: 0...100, step: 1)
            Text("\(Int(value))px").frame(width: 40)
            Button("✓") { onApply() }.font(.caption)
        }
    }
}
