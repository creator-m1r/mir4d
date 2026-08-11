// MirUIApp.swift — финальная версия с поддержкой тем, анимаций и таймером обновления.
//
// Теперь в меню «Вид» можно выбрать светлую или тёмную тему, и весь интерфейс
// мгновенно перестраивается под новую тему. Холст автоматически обновляется
// 60 раз в секунду для поддержки плавных анимаций.
//
// Чистый SwiftUI, использует C-функции моста.

import SwiftUI

@main
struct MirUIApp: App {
    @StateObject private var bridge = MirUIBridge.shared
    @StateObject private var canvasVM = DesignerCanvasViewModel()

    // Таймер для обновления холста (60 FPS)
    let timer = Timer.publish(every: 1.0 / 60.0, on: .main, in: .common).autoconnect()

    var body: some Scene {
        WindowGroup {
            HStack(spacing: 0) {
                DesignerCanvasView()
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                InspectorView(selectedWidgetId: $canvasVM.selectedWidgetId)
            }
            .frame(minWidth: 1060, minHeight: 600)
            .overlay(alignment: .top) { toolbarView }
            .onAppear { initializeAndRender() }
            .onReceive(timer) { _ in
                // Каждый тик таймера вызываем RenderFrame для обновления анимаций
                MirUI_RenderFrame()
            }
        }
        .commands {
            CommandGroup(after: .newItem) {
                Button("New") { MirUI_NewProject(); MirUI_RenderFrame() }
                    .keyboardShortcut("n")
                Button("Open...") { openProject() }
                    .keyboardShortcut("o")
                Button("Save") { saveProject() }
                    .keyboardShortcut("s")
            }
            CommandGroup(after: .undoRedo) {
                Button("Undo") { MirUI_Undo(); MirUI_RenderFrame() }
                    .keyboardShortcut("z", modifiers: .command)
                Button("Redo") { MirUI_Redo(); MirUI_RenderFrame() }
                    .keyboardShortcut("z", modifiers: [.command, .shift])
            }
            CommandGroup(after: .pasteboard) {
                Button("Copy") { canvasVM.copySelected() }
                    .keyboardShortcut("c")
                Button("Paste") { canvasVM.paste() }
                    .keyboardShortcut("v")
                Button("Cut") { canvasVM.cutSelected() }
                    .keyboardShortcut("x")
            }
            // Меню выбора темы
            CommandGroup(after: .sidebar) {
                Menu("Тема") {
                    Button("Светлая") { switchTheme("mir.light") }
                    Button("Тёмная")  { switchTheme("mir.dark") }
                }
            }
        }
    }

    var toolbarView: some View {
        HStack {
            Text("MirUI Designer").font(.headline).padding(.leading)
            Spacer()
            Button(action: { MirUI_NewProject(); MirUI_RenderFrame() }) {
                Image(systemName: "doc.badge.plus")
            }.help("Новый проект")
            Button(action: { openProject() }) {
                Image(systemName: "folder")
            }.help("Открыть проект")
            Button(action: { saveProject() }) {
                Image(systemName: "square.and.arrow.down")
            }.help("Сохранить проект")
            Divider().frame(height: 20)
            Menu("Добавить") {
                Button("Кнопка")            { addWidget("Button") }
                Button("Надпись")           { addWidget("Label") }
                Divider()
                Button("Флажок")            { addWidget("CheckBox") }
                Button("Текстовое поле")    { addWidget("TextField") }
                Button("Выпадающий список") { addWidget("ComboBox") }
                Button("Ползунок")          { addWidget("Slider") }
                Button("Радиокнопка")       { addWidget("RadioButton") }
                Button("Индикатор прогресса") { addWidget("ProgressBar") }
                Divider()
                Button("Изображение")       { addWidget("Image") }
                Button("Таблица")           { addWidget("TableView") }
                Button("Область прокрутки") { addWidget("ScrollView") }
                Button("Панель вкладок")    { addWidget("TabView") }
            }
            .padding(.horizontal, 8)

            Button("Анимировать") { demoAnimate() }
                .help("Продемонстрировать анимацию выделенного виджета")

            Divider().frame(height: 20)
            Button(action: { MirUI_Undo(); MirUI_RenderFrame() }) {
                Image(systemName: "arrow.uturn.backward")
            }.help("Отменить")
            Button(action: { MirUI_Redo(); MirUI_RenderFrame() }) {
                Image(systemName: "arrow.uturn.forward")
            }.help("Повторить")
            Divider().frame(height: 20)
            Button(action: { MirUI_TogglePreview(); MirUI_RenderFrame() }) {
                Image(systemName: "eye")
            }.help("Предпросмотр")
        }
        .padding(.vertical, 6)
        .padding(.horizontal)
        .background(.ultraThinMaterial)
    }

    func initializeAndRender() {
        MirUI_Init()
        MirUI_AddWidget("Button", 100, 100, 150, 40)
        MirUI_AddWidget("Label", 100, 200, 200, 30)
        MirUI_RenderFrame()
    }

    func addWidget(_ type: String) {
        let x = Double.random(in: 50...600)
        let y = Double.random(in: 50...500)
        MirUI_AddWidget(type.cString(using: .utf8), x, y, 160, 36)
        MirUI_RenderFrame()
    }

    func switchTheme(_ id: String) {
        MirUI_SwitchTheme(id.cString(using: .utf8))
        MirUI_RenderFrame()
    }

    func demoAnimate() {
        guard let widgetId = canvasVM.selectedWidgetIds.first else { return }
        let newX = Double.random(in: 50...500)
        let targetStr = String(Int(newX))
        MirUI_AnimateProperty(widgetId, "x", targetStr.cString(using: .utf8), 1.0, "spring")
    }

    func saveProject() {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.init(filenameExtension: "mirui")!]
        panel.nameFieldStringValue = "project.mirui"
        panel.begin { response in
            if response == .OK, let url = panel.url {
                let _ = MirUI_SaveProject(url.path.cString(using: .utf8))
            }
        }
    }

    func openProject() {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.init(filenameExtension: "mirui")!]
        panel.begin { response in
            if response == .OK, let url = panel.url {
                if MirUI_LoadProject(url.path.cString(using: .utf8)) {
                    canvasVM.selectedWidgetIds = []
                }
            }
        }
    }
}

// MARK: - C-функции (сигнатуры)

@_silgen_name("MirUI_Init") func MirUI_Init()
@_silgen_name("MirUI_AddWidget") func MirUI_AddWidget(_ type: UnsafePointer<CChar>?, _ x: Double, _ y: Double, _ w: Double, _ h: Double) -> Int64
@_silgen_name("MirUI_RenderFrame") func MirUI_RenderFrame()
@_silgen_name("MirUI_Undo") func MirUI_Undo()
@_silgen_name("MirUI_Redo") func MirUI_Redo()
@_silgen_name("MirUI_MoveWidget") func MirUI_MoveWidget(_ id: Int64, _ dx: Double, _ dy: Double)
@_silgen_name("MirUI_ResizeWidget") func MirUI_ResizeWidget(_ id: Int64, _ w: Double, _ h: Double, _ x: Double, _ y: Double)
@_silgen_name("MirUI_DeleteWidget") func MirUI_DeleteWidget(_ id: Int64)
@_silgen_name("MirUI_SetPropertyString") func MirUI_SetPropertyString(_ id: Int64, _ name: UnsafePointer<CChar>?, _ val: UnsafePointer<CChar>?)
@_silgen_name("MirUI_SetPropertyDouble") func MirUI_SetPropertyDouble(_ id: Int64, _ name: UnsafePointer<CChar>?, _ val: Double)
@_silgen_name("MirUI_SetPropertyBool") func MirUI_SetPropertyBool(_ id: Int64, _ name: UnsafePointer<CChar>?, _ val: Bool)
@_silgen_name("MirUI_GetPropertyString") func MirUI_GetPropertyString(_ id: Int64, _ name: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?
@_silgen_name("MirUI_CopyWidget") func MirUI_CopyWidget(_ id: Int64)
@_silgen_name("MirUI_PasteWidget") func MirUI_PasteWidget(_ parentId: Int64)
@_silgen_name("MirUI_CutWidget") func MirUI_CutWidget(_ id: Int64)
@_silgen_name("MirUI_AlignWidgets") func MirUI_AlignWidgets(_ ids: UnsafeRawPointer, _ count: Int32, _ strategy: UnsafePointer<CChar>)
@_silgen_name("MirUI_NewProject") func MirUI_NewProject()
@_silgen_name("MirUI_SaveProject") func MirUI_SaveProject(_ path: UnsafePointer<CChar>?) -> Bool
@_silgen_name("MirUI_LoadProject") func MirUI_LoadProject(_ path: UnsafePointer<CChar>?) -> Bool
@_silgen_name("MirUI_TogglePreview") func MirUI_TogglePreview()
@_silgen_name("MirUI_AnimateProperty") func MirUI_AnimateProperty(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ targetValue: UnsafePointer<CChar>?, _ duration: Double, _ curveType: UnsafePointer<CChar>?)
@_silgen_name("MirUI_SwitchTheme") func MirUI_SwitchTheme(_ themeId: UnsafePointer<CChar>?)
@_silgen_name("MirUI_CurrentThemeName") func MirUI_CurrentThemeName() -> UnsafePointer<CChar>?
@_silgen_name("MirUI_RegisterTheme") func MirUI_RegisterTheme(_ themeId: UnsafePointer<CChar>?)
@_silgen_name("MirUI_Shutdown") func MirUI_Shutdown()