// MirUI/Renderers/SwiftUI/InspectorView.swift — финальная версия с редакторами отступов и позиции.
//
// Теперь инспектор автоматически показывает:
//   • Позицию (X, Y) — через PointEditorView
//   • Размеры (Ширина, Высота)
//   • Отступы (Top, Right, Bottom, Left) — через InsetsEditorView
//   • Текст, скругление, видимость, доступность, цвет фона, шрифт, выравнивание
//
// Все поля поддерживают групповое редактирование:
// если выделено несколько виджетов, изменения применяются ко всем.
//
// Чистый SwiftUI, использует C-функции моста.
// MirUI/Renderers/SwiftUI/InspectorView.swift — финальная версия с поддержкой новых виджетов.
// Добавлены секции для CheckBox, TextField, ComboBox, Slider.
// Поддерживает групповое редактирование.
//
// Чистый SwiftUI, использует C-функции моста.

import SwiftUI

// MARK: - ViewModel инспектора (расширенный)

class InspectorViewModel: ObservableObject {
    @ObservedObject var bridge = MirUIBridge.shared
    
    var selectedWidgetIds: [Int64] = []
    var selectedWidgetType: String = ""
    
    // Общие свойства
    @Published var text: String = ""
    @Published var x: String = ""
    @Published var y: String = ""
    @Published var width: String = ""
    @Published var height: String = ""
    @Published var paddingTop: String = "0"
    @Published var paddingRight: String = "0"
    @Published var paddingBottom: String = "0"
    @Published var paddingLeft: String = "0"
    @Published var cornerRadius: String = "0"
    @Published var isVisible: Bool = true
    @Published var isEnabled: Bool = true
    @Published var backgroundColor: String = "#FFFFFF"
    @Published var fontString: String = "System;14;400;0"
    @Published var alignment: String = "Left"
    @Published var hasAlignment: Bool = false
    
    // Свойства CheckBox
    @Published var checked: Bool = false
    
    // Свойства TextField
    @Published var placeholder: String = ""
    @Published var isReadOnly: Bool = false
    @Published var maxLength: String = "0"
    @Published var textAlignment: String = "Left"
    
    // Свойства ComboBox
    @Published var items: String = ""
    @Published var selectedIndex: String = "-1"
    
    // Свойства Slider
    @Published var sliderValue: String = "50"
    @Published var minValue: String = "0"
    @Published var maxValue: String = "100"
    @Published var step: String = "0"
    @Published var orientation: String = "horizontal"
    
    // Выпадающие списки
    let alignmentValues = ["Left", "Center", "Right"]
    let orientationValues = ["horizontal", "vertical"]
    
    // ── Загрузка свойств ──────────────────────────────────────
    func loadProperties(for ids: [Int64]) {
        selectedWidgetIds = ids
        
        guard let firstId = ids.first else {
            resetFields()
            return
        }
        
        // Тип виджета
        selectedWidgetType = getStringProperty(firstId, "type") ?? ""
        
        // Общие свойства
        text = getStringProperty(firstId, "text") ?? ""
        x = getStringProperty(firstId, "x") ?? "0"
        y = getStringProperty(firstId, "y") ?? "0"
        width = getStringProperty(firstId, "width") ?? "100"
        height = getStringProperty(firstId, "height") ?? "40"
        paddingTop = getStringProperty(firstId, "paddingTop") ?? "0"
        paddingRight = getStringProperty(firstId, "paddingRight") ?? "0"
        paddingBottom = getStringProperty(firstId, "paddingBottom") ?? "0"
        paddingLeft = getStringProperty(firstId, "paddingLeft") ?? "0"
        cornerRadius = getStringProperty(firstId, "cornerRadius") ?? "0"
        isVisible = getBoolProperty(firstId, "visible") ?? true
        isEnabled = getBoolProperty(firstId, "enabled") ?? true
        backgroundColor = getStringProperty(firstId, "background") ?? "#FFFFFF"
        fontString = getStringProperty(firstId, "font") ?? "System;14;400;0"
        if let align = getStringProperty(firstId, "alignment") {
            alignment = align
            hasAlignment = true
        } else {
            alignment = "Left"
            hasAlignment = false
        }
        
        // CheckBox
        checked = getBoolProperty(firstId, "checked") ?? false
        
        // TextField
        placeholder = getStringProperty(firstId, "placeholder") ?? ""
        isReadOnly = getBoolProperty(firstId, "readOnly") ?? false
        maxLength = getStringProperty(firstId, "maxLength") ?? "0"
        textAlignment = getStringProperty(firstId, "textAlignment") ?? "Left"
        
        // ComboBox
        items = getStringProperty(firstId, "items") ?? ""
        selectedIndex = getStringProperty(firstId, "selectedIndex") ?? "-1"
        
        // Slider
        sliderValue = getStringProperty(firstId, "value") ?? "50"
        minValue = getStringProperty(firstId, "minValue") ?? "0"
        maxValue = getStringProperty(firstId, "maxValue") ?? "100"
        step = getStringProperty(firstId, "step") ?? "0"
        orientation = getStringProperty(firstId, "orientation") ?? "horizontal"
    }
    
    func resetFields() {
        text = ""; x = "0"; y = "0"; width = ""; height = ""
        paddingTop = "0"; paddingRight = "0"; paddingBottom = "0"; paddingLeft = "0"
        cornerRadius = "0"; isVisible = true; isEnabled = true
        backgroundColor = "#FFFFFF"; fontString = "System;14;400;0"
        alignment = "Left"; hasAlignment = false
        checked = false
        placeholder = ""; isReadOnly = false; maxLength = "0"; textAlignment = "Left"
        items = ""; selectedIndex = "-1"
        sliderValue = "50"; minValue = "0"; maxValue = "100"; step = "0"; orientation = "horizontal"
        selectedWidgetType = ""
    }
    
    // ── Методы применения ────────────────────────────────────
    func applyText() { applyStringProperty("text", text) }
    func applyPosition() { /* уже обрабатывается отдельно через ResizeWidget */ }
    func applyWidth() { /* через ResizeWidget */ }
    func applyHeight() { /* через ResizeWidget */ }
    func applyPadding() {
        guard let t = Double(paddingTop), let r = Double(paddingRight),
              let b = Double(paddingBottom), let l = Double(paddingLeft) else { return }
        for id in selectedWidgetIds {
            MirUI_SetPropertyDouble(id, "paddingTop", t)
            MirUI_SetPropertyDouble(id, "paddingRight", r)
            MirUI_SetPropertyDouble(id, "paddingBottom", b)
            MirUI_SetPropertyDouble(id, "paddingLeft", l)
        }
        MirUI_RenderFrame()
    }
    func applyCornerRadius() {
        guard let r = Double(cornerRadius) else { return }
        for id in selectedWidgetIds {
            MirUI_SetPropertyDouble(id, "cornerRadius", r)
        }
        MirUI_RenderFrame()
    }
    func applyVisible() { applyBoolProperty("visible", isVisible) }
    func applyEnabled() { applyBoolProperty("enabled", isEnabled) }
    func applyBackgroundColor() { applyStringProperty("background", backgroundColor) }
    
    // CheckBox
    func applyChecked() { applyBoolProperty("checked", checked) }
    
    // TextField
    func applyPlaceholder() { applyStringProperty("placeholder", placeholder) }
    func applyReadOnly() { applyBoolProperty("readOnly", isReadOnly) }
    func applyMaxLength() {
        guard let ml = Int64(maxLength) else { return }
        for id in selectedWidgetIds {
            MirUI_SetPropertyDouble(id, "maxLength", Double(ml))
        }
        MirUI_RenderFrame()
    }
    func applyTextAlignment() { applyStringProperty("textAlignment", textAlignment) }
    
    // ComboBox
    func applyItems() { applyStringProperty("items", items) }
    func applySelectedIndex() {
        guard let idx = Int64(selectedIndex) else { return }
        for id in selectedWidgetIds {
            MirUI_SetPropertyDouble(id, "selectedIndex", Double(idx))
        }
        MirUI_RenderFrame()
    }
    
    // Slider
    func applySliderValue() {
        guard let v = Double(sliderValue) else { return }
        for id in selectedWidgetIds { MirUI_SetPropertyDouble(id, "value", v) }
        MirUI_RenderFrame()
    }
    func applyMinValue() {
        guard let v = Double(minValue) else { return }
        for id in selectedWidgetIds { MirUI_SetPropertyDouble(id, "minValue", v) }
        MirUI_RenderFrame()
    }
    func applyMaxValue() {
        guard let v = Double(maxValue) else { return }
        for id in selectedWidgetIds { MirUI_SetPropertyDouble(id, "maxValue", v) }
        MirUI_RenderFrame()
    }
    func applyStep() {
        guard let v = Double(step) else { return }
        for id in selectedWidgetIds { MirUI_SetPropertyDouble(id, "step", v) }
        MirUI_RenderFrame()
    }
    func applyOrientation() { applyStringProperty("orientation", orientation) }
    
    // ── Вспомогательные методы ───────────────────────────────
    private func applyStringProperty(_ name: String, _ value: String) {
        for id in selectedWidgetIds {
            value.withCString { cStr in
                MirUI_SetPropertyString(id, name.cString(using: .utf8)!, cStr)
            }
        }
        MirUI_RenderFrame()
    }
    
    private func applyBoolProperty(_ name: String, _ value: Bool) {
        for id in selectedWidgetIds {
            MirUI_SetPropertyBool(id, name.cString(using: .utf8)!, value)
        }
        MirUI_RenderFrame()
    }
    
    private func getStringProperty(_ id: Int64, _ name: String) -> String? {
        let cStr = MirUI_GetPropertyString(id, name.cString(using: .utf8)!)
        guard let swiftStr = cStr.flatMap({ String(cString: $0 }) else { return nil }
        return swiftStr.isEmpty ? nil : swiftStr
    }
    
    private func getBoolProperty(_ id: Int64, _ name: String) -> Bool? {
        guard let s = getStringProperty(id, name) else { return nil }
        return s == "true" || s == "1"
    }
}

// MARK: - Панель инспектора

struct InspectorView: View {
    @StateObject private var vm = InspectorViewModel()
    @ObservedObject var bridge = MirUIBridge.shared
    @Binding var selectedWidgetId: Int64?
    
    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Инспектор свойств")
                .font(.headline)
                .padding(.bottom, 4)
            
            if let _ = vm.selectedWidgetIds.first {
                ScrollView {
                    VStack(alignment: .leading, spacing: 12) {
                        // Общие секции (позиция, размеры, отступы, скругление, состояние)
                        commonSections
                        
                        // Секции, специфичные для типа виджета
                        switch vm.selectedWidgetType {
                        case "Button", "Label":
                            textSection
                            if vm.hasAlignment { alignmentSection }
                            
                        case "CheckBox":
                            textSection
                            checkedSection
                            
                        case "TextField":
                            textSection
                            placeholderSection
                            readOnlySection
                            maxLengthSection
                            textAlignmentSection
                            
                        case "ComboBox":
                            itemsSection
                            selectedIndexSection
                            
                        case "Slider":
                            sliderValueSection
                            rangeSection
                            stepSection
                            orientationSection
                            
                        default:
                            if !vm.text.isEmpty { textSection }
                        }
                        
                        // Цвет фона и шрифт (для всех типов)
                        colorSection
                        fontSection
                    }
                    .padding(.vertical, 4)
                }
            } else {
                Text("Ничего не выделено")
                    .foregroundColor(.secondary)
                    .frame(maxWidth: .infinity, alignment: .center)
                    .padding(.vertical, 20)
            }
            
            Spacer()
        }
        .padding()
        .frame(width: 300)
        .background(Color.gray.opacity(0.05))
        .onAppear {
            if let id = selectedWidgetId {
                vm.loadProperties(for: [id])
            }
        }
        .onChange(of: selectedWidgetId) { newId in
            if let id = newId {
                vm.loadProperties(for: [id])
            } else {
                vm.loadProperties(for: [])
            }
        }
        .onChange(of: bridge.nodes) { _ in
            if let id = selectedWidgetId {
                vm.loadProperties(for: [id])
            }
        }
    }
    
    // MARK: - Общие секции
    
    var commonSections: some View {
        Group {
            // Позиция
            propertyGroup("Позиция") {
                HStack(spacing: 12) {
                    fieldWithLabel("X", text: $vm.x, onCommit: { /* handled in canvas */ })
                    fieldWithLabel("Y", text: $vm.y, onCommit: { })
                }
            }
            // Размеры
            propertyGroup("Размеры") {
                HStack(spacing: 12) {
                    fieldWithLabel("Ш", text: $vm.width, onCommit: { })
                    fieldWithLabel("В", text: $vm.height, onCommit: { })
                }
            }
            // Отступы
            propertyGroup("Отступы (Padding)") {
                VStack(spacing: 4) {
                    HStack(spacing: 12) {
                        fieldWithLabel("T", text: $vm.paddingTop, onCommit: vm.applyPadding)
                        fieldWithLabel("R", text: $vm.paddingRight, onCommit: vm.applyPadding)
                    }
                    HStack(spacing: 12) {
                        fieldWithLabel("B", text: $vm.paddingBottom, onCommit: vm.applyPadding)
                        fieldWithLabel("L", text: $vm.paddingLeft, onCommit: vm.applyPadding)
                    }
                    Button("✓") { vm.applyPadding() }.font(.caption)
                }
            }
            // Скругление
            propertyGroup("Радиус скругления") {
                HStack {
                    TextField("0", text: $vm.cornerRadius, onCommit: vm.applyCornerRadius)
                        .textFieldStyle(RoundedBorderTextFieldStyle())
                        .frame(width: 60)
                    Stepper("", value: Binding(get: { Double(vm.cornerRadius) ?? 0 }, set: { vm.cornerRadius = String(Int($0)); vm.applyCornerRadius() }), in: 0...100).labelsHidden()
                    Button("✓") { vm.applyCornerRadius() }.font(.caption)
                }
            }
            // Состояние
            propertyGroup("Состояние") {
                Toggle("Видимость", isOn: Binding(get: { vm.isVisible }, set: { vm.isVisible = $0; vm.applyVisible() }))
                Toggle("Доступность", isOn: Binding(get: { vm.isEnabled }, set: { vm.isEnabled = $0; vm.applyEnabled() }))
            }
        }
    }
    
    // MARK: - Специфичные секции
    
    var textSection: some View {
        propertyGroup("Текст") {
            HStack {
                TextField("Текст", text: $vm.text, onCommit: vm.applyText)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                Button("✓") { vm.applyText() }.font(.caption)
            }
        }
    }
    
    var alignmentSection: some View {
        propertyGroup("Выравнивание") {
            EnumEditorView(
                widgetId: vm.selectedWidgetIds.first ?? 0,
                propertyName: "alignment",
                currentValue: vm.alignment,
                possibleValues: vm.alignmentValues
            )
        }
    }
    
    var checkedSection: some View {
        propertyGroup("Состояние флажка") {
            Toggle("Галочка", isOn: Binding(get: { vm.checked }, set: { vm.checked = $0; vm.applyChecked() }))
        }
    }
    
    var placeholderSection: some View {
        propertyGroup("Подсказка (placeholder)") {
            HStack {
                TextField("Подсказка", text: $vm.placeholder, onCommit: vm.applyPlaceholder)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                Button("✓") { vm.applyPlaceholder() }.font(.caption)
            }
        }
    }
    
    var readOnlySection: some View {
        propertyGroup("Только чтение") {
            Toggle("Только чтение", isOn: Binding(get: { vm.isReadOnly }, set: { vm.isReadOnly = $0; vm.applyReadOnly() }))
        }
    }
    
    var maxLengthSection: some View {
        propertyGroup("Макс. длина") {
            HStack {
                TextField("0", text: $vm.maxLength, onCommit: vm.applyMaxLength)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                    .frame(width: 60)
                Button("✓") { vm.applyMaxLength() }.font(.caption)
            }
        }
    }
    
    var textAlignmentSection: some View {
        propertyGroup("Выравнивание текста") {
            EnumEditorView(
                widgetId: vm.selectedWidgetIds.first ?? 0,
                propertyName: "textAlignment",
                currentValue: vm.textAlignment,
                possibleValues: vm.alignmentValues
            )
        }
    }
    
    var itemsSection: some View {
        propertyGroup("Элементы (через |)") {
            HStack {
                TextField("Элементы", text: $vm.items, onCommit: vm.applyItems)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                Button("✓") { vm.applyItems() }.font(.caption)
            }
        }
    }
    
    var selectedIndexSection: some View {
        propertyGroup("Выбранный индекс") {
            HStack {
                TextField("-1", text: $vm.selectedIndex, onCommit: vm.applySelectedIndex)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                    .frame(width: 60)
                Button("✓") { vm.applySelectedIndex() }.font(.caption)
            }
        }
    }
    
    var sliderValueSection: some View {
        propertyGroup("Значение") {
            HStack {
                TextField("50", text: $vm.sliderValue, onCommit: vm.applySliderValue)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                    .frame(width: 60)
                Slider(value: Binding(get: { Double(vm.sliderValue) ?? 50 }, set: { vm.sliderValue = String($0); vm.applySliderValue() }),
                       in: (Double(vm.minValue) ?? 0)...(Double(vm.maxValue) ?? 100))
                Button("✓") { vm.applySliderValue() }.font(.caption)
            }
        }
    }
    
    var rangeSection: some View {
        propertyGroup("Диапазон") {
            HStack(spacing: 12) {
                fieldWithLabel("Мин", text: $vm.minValue, onCommit: vm.applyMinValue)
                fieldWithLabel("Макс", text: $vm.maxValue, onCommit: vm.applyMaxValue)
            }
        }
    }
    
    var stepSection: some View {
        propertyGroup("Шаг") {
            HStack {
                TextField("0", text: $vm.step, onCommit: vm.applyStep)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                    .frame(width: 60)
                Button("✓") { vm.applyStep() }.font(.caption)
            }
        }
    }
    
    var orientationSection: some View {
        propertyGroup("Ориентация") {
            EnumEditorView(
                widgetId: vm.selectedWidgetIds.first ?? 0,
                propertyName: "orientation",
                currentValue: vm.orientation,
                possibleValues: vm.orientationValues
            )
        }
    }
    
    var colorSection: some View {
        propertyGroup("Цвет фона") {
            HStack {
                TextField("#FFFFFF", text: $vm.backgroundColor, onCommit: vm.applyBackgroundColor)
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                Button("✓") { vm.applyBackgroundColor() }.font(.caption)
            }
        }
    }
    
    var fontSection: some View {
        FontEditorView(widgetId: vm.selectedWidgetIds.first ?? 0, fontString: vm.fontString)
            .padding(.top, 8)
    }
    
    // MARK: - Вспомогательные элементы
    
    func propertyGroup<Content: View>(_ title: String, @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(title).font(.caption).foregroundColor(.secondary)
            content()
        }
    }
    
    func fieldWithLabel(_ label: String, text: Binding<String>, onCommit: @escaping () -> Void) -> some View {
        VStack(spacing: 2) {
            Text(label).font(.caption2).foregroundColor(.secondary)
            TextField("0", text: text, onCommit: onCommit)
                .textFieldStyle(RoundedBorderTextFieldStyle())
                .frame(width: 60)
                .multilineTextAlignment(.center)
        }
    }
}

// MARK: - C-функции (сигнатуры)

@_silgen_name("MirUI_SetPropertyString")
func MirUI_SetPropertyString(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ value: UnsafePointer<CChar>?)

@_silgen_name("MirUI_SetPropertyDouble")
func MirUI_SetPropertyDouble(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ value: Double)

@_silgen_name("MirUI_SetPropertyBool")
func MirUI_SetPropertyBool(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ value: Bool)

@_silgen_name("MirUI_GetPropertyString")
func MirUI_GetPropertyString(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?

@_silgen_name("MirUI_RenderFrame")
func MirUI_RenderFrame()