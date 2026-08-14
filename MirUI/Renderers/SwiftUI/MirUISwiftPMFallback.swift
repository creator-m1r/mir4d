import Foundation

// MARK: - SwiftPM fallback for the MirUI C ABI
//
// The native MirUI implementation lives in MirUICppBridge.mm. SwiftPM cannot
// compile Objective-C++ sources inside the Swift-only MIR4DApp target, so the
// package target needs a link-safe implementation of the same C symbols.
//
// This file is intentionally a fallback only. The native Xcode/macOS target
// must continue to link MirUICppBridge.mm and therefore uses the real MirUI
// implementation. SwiftPM builds use these no-op/default implementations so
// the Swift application can be compiled and tested without directly linking
// the internal SwiftUICore framework.

@_cdecl("MirUI_Init")
public func mirUIInit() {}

@_cdecl("MirUI_Shutdown")
public func mirUIShutdown() {}

@_cdecl("MirUI_AddWidget")
public func mirUIAddWidget(
    _ widgetType: UnsafePointer<CChar>?,
    _ x: Double,
    _ y: Double,
    _ w: Double,
    _ h: Double
) -> Int64 {
    _ = widgetType
    _ = x
    _ = y
    _ = w
    _ = h
    return 0
}

@_cdecl("MirUI_AddButton")
public func mirUIAddButton(
    _ text: UnsafePointer<CChar>?,
    _ x: Double,
    _ y: Double,
    _ w: Double,
    _ h: Double
) {
    _ = text
    _ = x
    _ = y
    _ = w
    _ = h
}

@_cdecl("MirUI_MoveWidget")
public func mirUIMoveWidget(_ widgetId: Int64, _ dx: Double, _ dy: Double) {
    _ = widgetId
    _ = dx
    _ = dy
}

@_cdecl("MirUI_ResizeWidget")
public func mirUIResizeWidget(
    _ widgetId: Int64,
    _ newWidth: Double,
    _ newHeight: Double,
    _ newX: Double,
    _ newY: Double
) {
    _ = widgetId
    _ = newWidth
    _ = newHeight
    _ = newX
    _ = newY
}

@_cdecl("MirUI_DeleteWidget")
public func mirUIDeleteWidget(_ widgetId: Int64) {
    _ = widgetId
}

@_cdecl("MirUI_SetPropertyString")
public func mirUISetPropertyString(
    _ widgetId: Int64,
    _ propertyName: UnsafePointer<CChar>?,
    _ value: UnsafePointer<CChar>?
) {
    _ = widgetId
    _ = propertyName
    _ = value
}

@_cdecl("MirUI_SetPropertyDouble")
public func mirUISetPropertyDouble(
    _ widgetId: Int64,
    _ propertyName: UnsafePointer<CChar>?,
    _ value: Double
) {
    _ = widgetId
    _ = propertyName
    _ = value
}

@_cdecl("MirUI_SetPropertyBool")
public func mirUISetPropertyBool(
    _ widgetId: Int64,
    _ propertyName: UnsafePointer<CChar>?,
    _ value: Bool
) {
    _ = widgetId
    _ = propertyName
    _ = value
}

@_cdecl("MirUI_GetPropertyString")
public func mirUIGetPropertyString(
    _ widgetId: Int64,
    _ propertyName: UnsafePointer<CChar>?
) -> UnsafePointer<CChar>? {
    _ = widgetId
    _ = propertyName
    return strdup("")
}

@_cdecl("MirUI_SetProperty")
public func mirUISetProperty(
    _ widgetId: Int64,
    _ propertyName: UnsafePointer<CChar>?,
    _ value: UnsafePointer<CChar>?
) {
    _ = widgetId
    _ = propertyName
    _ = value
}

@_cdecl("MirUI_RenderFrame")
public func mirUIRenderFrame() {}

@_cdecl("MirUI_Undo")
public func mirUIUndo() {}

@_cdecl("MirUI_Redo")
public func mirUIRedo() {}

@_cdecl("MirUI_SelectWidget")
public func mirUISelectWidget(_ widgetId: Int64) {
    _ = widgetId
}

@_cdecl("MirUI_ClearSelection")
public func mirUIClearSelection() {}

@_cdecl("MirUI_CopyWidget")
public func mirUICopyWidget(_ widgetId: Int64) {
    _ = widgetId
}

@_cdecl("MirUI_PasteWidget")
public func mirUIPasteWidget(_ parentId: Int64) {
    _ = parentId
}

@_cdecl("MirUI_CutWidget")
public func mirUICutWidget(_ widgetId: Int64) {
    _ = widgetId
}

@_cdecl("MirUI_AlignWidgets")
public func mirUIAlignWidgets(
    _ widgetIds: UnsafeRawPointer?,
    _ count: Int32,
    _ strategy: UnsafePointer<CChar>?
) {
    _ = widgetIds
    _ = count
    _ = strategy
}

@_cdecl("MirUI_NewProject")
public func mirUINewProject() {}

@_cdecl("MirUI_SaveProject")
public func mirUISaveProject(_ path: UnsafePointer<CChar>?) -> Bool {
    _ = path
    return false
}

@_cdecl("MirUI_LoadProject")
public func mirUILoadProject(_ path: UnsafePointer<CChar>?) -> Bool {
    _ = path
    return false
}

@_cdecl("MirUI_EnterPreview")
public func mirUIEnterPreview() {}

@_cdecl("MirUI_ExitPreview")
public func mirUIExitPreview() {}

@_cdecl("MirUI_TogglePreview")
public func mirUITogglePreview() {}

@_cdecl("MirUI_SwitchTheme")
public func mirUISwitchTheme(_ themeId: UnsafePointer<CChar>?) {
    _ = themeId
}

@_cdecl("MirUI_CurrentThemeName")
public func mirUICurrentThemeName() -> UnsafePointer<CChar>? {
    strdup("SwiftPM fallback")
}

@_cdecl("MirUI_RegisterTheme")
public func mirUIRegisterTheme(_ themeId: UnsafePointer<CChar>?) {
    _ = themeId
}

@_cdecl("MirUI_GetThemeColor")
public func mirUIGetThemeColor(_ colorToken: UnsafePointer<CChar>?) -> UnsafePointer<CChar>? {
    _ = colorToken
    return strdup("#000000FF")
}

@_cdecl("MirUI_SetThemeColor")
public func mirUISetThemeColor(
    _ colorToken: UnsafePointer<CChar>?,
    _ hexColor: UnsafePointer<CChar>?
) {
    _ = colorToken
    _ = hexColor
}

@_cdecl("MirUI_GetThemeMetric")
public func mirUIGetThemeMetric(_ metricToken: UnsafePointer<CChar>?) -> Double {
    _ = metricToken
    return 0.0
}

@_cdecl("MirUI_SetThemeMetric")
public func mirUISetThemeMetric(_ metricToken: UnsafePointer<CChar>?, _ value: Double) {
    _ = metricToken
    _ = value
}

@_cdecl("MirUI_GetThemeFont")
public func mirUIGetThemeFont(_ fontToken: UnsafePointer<CChar>?) -> UnsafePointer<CChar>? {
    _ = fontToken
    return strdup("system")
}

@_cdecl("MirUI_SetThemeFont")
public func mirUISetThemeFont(
    _ fontToken: UnsafePointer<CChar>?,
    _ fontString: UnsafePointer<CChar>?
) {
    _ = fontToken
    _ = fontString
}

@_cdecl("MirUI_GetThemeAnimationDuration")
public func mirUIGetThemeAnimationDuration() -> Double {
    0.0
}

@_cdecl("MirUI_SetThemeAnimationDuration")
public func mirUISetThemeAnimationDuration(_ duration: Double) {
    _ = duration
}

@_cdecl("MirUI_GetWidgetStyleField")
public func mirUIGetWidgetStyleField(
    _ widgetType: UnsafePointer<CChar>?,
    _ widgetState: UnsafePointer<CChar>?,
    _ fieldName: UnsafePointer<CChar>?
) -> UnsafePointer<CChar>? {
    _ = widgetType
    _ = widgetState
    _ = fieldName
    return strdup("")
}

@_cdecl("MirUI_SetWidgetStyleField")
public func mirUISetWidgetStyleField(
    _ widgetType: UnsafePointer<CChar>?,
    _ widgetState: UnsafePointer<CChar>?,
    _ fieldName: UnsafePointer<CChar>?,
    _ value: UnsafePointer<CChar>?
) {
    _ = widgetType
    _ = widgetState
    _ = fieldName
    _ = value
}

@_cdecl("MirUI_SwiftUI_UpdateViewNodes")
public func mirUISwiftUIUpdateViewNodes(
    _ nodes: UnsafeRawPointer?,
    _ count: Int32,
    _ rootIndex: Int32
) {
    _ = nodes
    _ = count
    _ = rootIndex
}

@_cdecl("MirUI_ExecuteCommand")
public func mirUIExecuteCommand(_ commandId: UnsafePointer<CChar>, _ widgetId: Int64) {
    _ = commandId
    _ = widgetId
}
