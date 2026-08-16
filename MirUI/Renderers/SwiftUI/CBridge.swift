import Foundation
import SwiftUI

// MARK: - MIR 4D UI C ABI
// These declarations are required by both the Xcode/macOS target and the
// SwiftPM target. They must not disappear when MIR4D_SWIFTPM is defined.
@_silgen_name("MirUI_RenderFrame") public func MirUI_RenderFrame()
@_silgen_name("MirUI_NewProject") public func MirUI_NewProject()
@_silgen_name("MirUI_ResizeWidget") public func MirUI_ResizeWidget(_ widgetId: Int64, _ newWidth: Double, _ newHeight: Double, _ newX: Double, _ newY: Double)
@_silgen_name("MirUI_MoveWidget") public func MirUI_MoveWidget(_ widgetId: Int64, _ dx: Double, _ dy: Double)
@_silgen_name("MirUI_DeleteWidget") public func MirUI_DeleteWidget(_ widgetId: Int64)
@_silgen_name("MirUI_CopyWidget") public func MirUI_CopyWidget(_ widgetId: Int64)
@_silgen_name("MirUI_PasteWidget") public func MirUI_PasteWidget(_ parentId: Int64)
@_silgen_name("MirUI_CutWidget") public func MirUI_CutWidget(_ widgetId: Int64)
@_silgen_name("MirUI_Undo") public func MirUI_Undo()
@_silgen_name("MirUI_Redo") public func MirUI_Redo()
@_silgen_name("MirUI_AlignWidgets") public func MirUI_AlignWidgets(_ widgetIds: UnsafeRawPointer, _ count: Int32, _ strategy: UnsafePointer<CChar>)
@_silgen_name("MirUI_SetPropertyString") public func MirUI_SetPropertyString(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ value: UnsafePointer<CChar>?)
@_silgen_name("MirUI_GetPropertyString") public func MirUI_GetPropertyString(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?
@_silgen_name("MirUI_SetPropertyDouble") public func MirUI_SetPropertyDouble(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ value: Double)
@_silgen_name("MirUI_SetPropertyBool") public func MirUI_SetPropertyBool(_ widgetId: Int64, _ propertyName: UnsafePointer<CChar>?, _ value: Bool)
@_silgen_name("MirUI_GetThemeColor") public func MirUI_GetThemeColor(_ colorToken: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?
@_silgen_name("MirUI_SetThemeColor") public func MirUI_SetThemeColor(_ colorToken: UnsafePointer<CChar>?, _ hexColor: UnsafePointer<CChar>?)
@_silgen_name("MirUI_GetThemeMetric") public func MirUI_GetThemeMetric(_ metricToken: UnsafePointer<CChar>?) -> Double
@_silgen_name("MirUI_SetThemeMetric") public func MirUI_SetThemeMetric(_ metricToken: UnsafePointer<CChar>?, _ value: Double)
@_silgen_name("MirUI_GetThemeFont") public func MirUI_GetThemeFont(_ fontToken: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?
@_silgen_name("MirUI_SetThemeFont") public func MirUI_SetThemeFont(_ fontToken: UnsafePointer<CChar>?, _ fontString: UnsafePointer<CChar>?)
@_silgen_name("MirUI_GetThemeAnimationDuration") public func MirUI_GetThemeAnimationDuration() -> Double
@_silgen_name("MirUI_SetThemeAnimationDuration") public func MirUI_SetThemeAnimationDuration(_ duration: Double)
@_silgen_name("MirUI_SwitchTheme") public func MirUI_SwitchTheme(_ themeId: UnsafePointer<CChar>?)
@_silgen_name("MirUI_CurrentThemeName") public func MirUI_CurrentThemeName() -> UnsafePointer<CChar>?
@_silgen_name("MirUI_GetWidgetStyleField") public func MirUI_GetWidgetStyleField(_ widgetType: UnsafePointer<CChar>?, _ widgetState: UnsafePointer<CChar>?, _ fieldName: UnsafePointer<CChar>?) -> UnsafePointer<CChar>?
@_silgen_name("MirUI_SetWidgetStyleField") public func MirUI_SetWidgetStyleField(_ widgetType: UnsafePointer<CChar>?, _ widgetState: UnsafePointer<CChar>?, _ fieldName: UnsafePointer<CChar>?, _ value: UnsafePointer<CChar>?)
@_silgen_name("MirUI_ExecuteCommand") public func MirUI_ExecuteCommand(_ commandId: UnsafePointer<CChar>, _ widgetId: Int64)

// MARK: - MirEngine OpenGL ABI
#if MIR4D_SWIFTPM
public struct MirEngineSize2D {
    public var width: UInt32
    public var height: UInt32
    public init(width: UInt32, height: UInt32) { self.width = width; self.height = height }
}
public func MirEngineCreateMacOpenGLContext(_ view: UnsafeMutableRawPointer?, _ size: MirEngineSize2D) -> UnsafeMutableRawPointer? { nil }
public func MirEngineDestroyOpenGLContext(_ context: UnsafeMutableRawPointer?) {}
public func MirEngineCreateOpenGLRenderer(_ context: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer? { nil }
public func MirEngineInitializeRenderer(_ renderer: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineDestroyRenderer(_ renderer: UnsafeMutableRawPointer?) {}
public func MirEngineCreateViewport(_ renderer: UnsafeMutableRawPointer?, _ width: UInt32, _ height: UInt32) -> UnsafeMutableRawPointer? { nil }
public func MirEngineDestroyViewport(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineRender(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineResize(_ viewport: UnsafeMutableRawPointer?, _ width: UInt32, _ height: UInt32) {}
public func MirEngineCreateBox(_ viewport: UnsafeMutableRawPointer?, _ width: Double, _ depth: Double, _ height: Double, _ objectId: UnsafeMutablePointer<UInt64>?) -> Bool { false }
public func MirEngineImportMesh(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?) -> Bool { false }
public func MirEngineGetLastError(_ viewport: UnsafeMutableRawPointer?) -> UnsafePointer<CChar>? { nil }
public func MirEngineExportStl(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?, _ selectionOnly: Bool) -> Bool { false }
public func MirEngineGetSelectedObjectId(_ viewport: UnsafeMutableRawPointer?) -> UInt64 { 0 }
public func MirEngineGetCameraOrientation(_ viewport: UnsafeMutableRawPointer?, _ theta: UnsafeMutablePointer<Float>?, _ phi: UnsafeMutablePointer<Float>?, _ distance: UnsafeMutablePointer<Float>?) {}
public func MirEngineSetCameraOrientation(_ viewport: UnsafeMutableRawPointer?, _ theta: Float, _ phi: Float, _ distance: Float) {}
public func MirEngineSetCameraProjection(_ viewport: UnsafeMutableRawPointer?, _ projection: Int32) {}
public func MirEngineGetCameraProjection(_ viewport: UnsafeMutableRawPointer?) -> Int32 { 0 }
public func MirEngineSetCameraFov(_ viewport: UnsafeMutableRawPointer?, _ fovYRadians: Float) {}
public func MirEngineSetActiveCameraPreset(_ viewport: UnsafeMutableRawPointer?, _ preset: Int32) {}
public func MirEngineFitViewport(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineGetOpenGLDiagnostics(_ renderer: UnsafeMutableRawPointer?, _ buffer: UnsafeMutablePointer<CChar>?, _ bufferSize: Int) -> Bool { false }
public func MirEngineViewportMouseDown(_ viewport: UnsafeMutableRawPointer?, _ button: Int32, _ x: Float, _ y: Float) {}
public func MirEngineViewportMouseUp(_ viewport: UnsafeMutableRawPointer?, _ button: Int32, _ x: Float, _ y: Float) {}
public func MirEngineViewportMouseMove(_ viewport: UnsafeMutableRawPointer?, _ x: Float, _ y: Float) {}
public func MirEngineViewportScroll(_ viewport: UnsafeMutableRawPointer?, _ delta: Float) {}
public func MirEngineViewportZoomAt(_ viewport: UnsafeMutableRawPointer?, _ delta: Float, _ x: Float, _ y: Float) {}
public func MirEngineViewportPan(_ viewport: UnsafeMutableRawPointer?, _ dx: Float, _ dy: Float) {}
public func MirEngineViewportOrbit(_ viewport: UnsafeMutableRawPointer?, _ dx: Float, _ dy: Float) {}
public func MirEngineViewportClick(_ viewport: UnsafeMutableRawPointer?, _ x: Float, _ y: Float, _ addToSelection: Bool) {}
public func MirEngineViewportHover(_ viewport: UnsafeMutableRawPointer?, _ x: Float, _ y: Float) {}
public func MirEngineViewportHoverClear(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineDeleteSelectedObject(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineClearSelection(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineViewportDragCancel(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineUndo(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineRedo(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineCanUndo(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineCanRedo(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
#else
@_silgen_name("MirEngineCreateMacOpenGLContext") public func MirEngineCreateMacOpenGLContext(_ view: UnsafeMutableRawPointer?, _ size: MirEngineSize2D) -> UnsafeMutableRawPointer?
@_silgen_name("MirEngineDestroyOpenGLContext") public func MirEngineDestroyOpenGLContext(_ context: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineCreateOpenGLRenderer") public func MirEngineCreateOpenGLRenderer(_ context: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer?
@_silgen_name("MirEngineInitializeRenderer") public func MirEngineInitializeRenderer(_ renderer: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineDestroyRenderer") public func MirEngineDestroyRenderer(_ renderer: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineCreateViewport") public func MirEngineCreateViewport(_ renderer: UnsafeMutableRawPointer?, _ width: UInt32, _ height: UInt32) -> UnsafeMutableRawPointer?
@_silgen_name("MirEngineDestroyViewport") public func MirEngineDestroyViewport(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineRender") public func MirEngineRender(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineResize") public func MirEngineResize(_ viewport: UnsafeMutableRawPointer?, _ width: UInt32, _ height: UInt32)
@_silgen_name("MirEngineCreateBox") public func MirEngineCreateBox(_ viewport: UnsafeMutableRawPointer?, _ width: Double, _ depth: Double, _ height: Double, _ objectId: UnsafeMutablePointer<UInt64>?) -> Bool
@_silgen_name("MirEngineImportMesh") public func MirEngineImportMesh(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?) -> Bool
@_silgen_name("MirEngineGetLastError") public func MirEngineGetLastError(_ viewport: UnsafeMutableRawPointer?) -> UnsafePointer<CChar>?
@_silgen_name("MirEngineExportStl") public func MirEngineExportStl(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?, _ selectionOnly: Bool) -> Bool
@_silgen_name("MirEngineMaterialCount") public func MirEngineMaterialCount() -> Int32
@_silgen_name("MirEngineMaterialName") public func MirEngineMaterialName(_ materialId: Int32, _ buffer: UnsafeMutablePointer<CChar>?, _ bufferSize: Int) -> Bool
@_silgen_name("MirEngineSetObjectMaterial") public func MirEngineSetObjectMaterial(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64, _ materialId: Int32) -> Bool
@_silgen_name("MirEngineGetOpenGLDiagnostics") public func MirEngineGetOpenGLDiagnostics(_ renderer: UnsafeMutableRawPointer?, _ buffer: UnsafeMutablePointer<CChar>?, _ bufferSize: Int) -> Bool
@_silgen_name("MirEngineGetSelectedObjectId") public func MirEngineGetSelectedObjectId(_ viewport: UnsafeMutableRawPointer?) -> UInt64
@_silgen_name("MirEngineGetCameraOrientation") public func MirEngineGetCameraOrientation(_ viewport: UnsafeMutableRawPointer?, _ theta: UnsafeMutablePointer<Float>?, _ phi: UnsafeMutablePointer<Float>?, _ distance: UnsafeMutablePointer<Float>?)
@_silgen_name("MirEngineSetCameraOrientation") public func MirEngineSetCameraOrientation(_ viewport: UnsafeMutableRawPointer?, _ theta: Float, _ phi: Float, _ distance: Float)
@_silgen_name("MirEngineSetCameraProjection") public func MirEngineSetCameraProjection(_ viewport: UnsafeMutableRawPointer?, _ projection: Int32)
@_silgen_name("MirEngineGetCameraProjection") public func MirEngineGetCameraProjection(_ viewport: UnsafeMutableRawPointer?) -> Int32
@_silgen_name("MirEngineSetCameraFov") public func MirEngineSetCameraFov(_ viewport: UnsafeMutableRawPointer?, _ fovYRadians: Float)
@_silgen_name("MirEngineSetActiveCameraPreset") public func MirEngineSetActiveCameraPreset(_ viewport: UnsafeMutableRawPointer?, _ preset: Int32)
@_silgen_name("MirEngineFitViewport") public func MirEngineFitViewport(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineViewportMouseDown") public func MirEngineViewportMouseDown(_ viewport: UnsafeMutableRawPointer?, _ button: Int32, _ x: Float, _ y: Float)
@_silgen_name("MirEngineViewportMouseUp") public func MirEngineViewportMouseUp(_ viewport: UnsafeMutableRawPointer?, _ button: Int32, _ x: Float, _ y: Float)
@_silgen_name("MirEngineViewportMouseMove") public func MirEngineViewportMouseMove(_ viewport: UnsafeMutableRawPointer?, _ x: Float, _ y: Float)
@_silgen_name("MirEngineViewportScroll") public func MirEngineViewportScroll(_ viewport: UnsafeMutableRawPointer?, _ delta: Float)
@_silgen_name("MirEngineViewportZoomAt") public func MirEngineViewportZoomAt(_ viewport: UnsafeMutableRawPointer?, _ delta: Float, _ x: Float, _ y: Float)
@_silgen_name("MirEngineViewportPan") public func MirEngineViewportPan(_ viewport: UnsafeMutableRawPointer?, _ dx: Float, _ dy: Float)
@_silgen_name("MirEngineViewportOrbit") public func MirEngineViewportOrbit(_ viewport: UnsafeMutableRawPointer?, _ dx: Float, _ dy: Float)
@_silgen_name("MirEngineViewportClick") public func MirEngineViewportClick(_ viewport: UnsafeMutableRawPointer?, _ x: Float, _ y: Float, _ addToSelection: Bool)
@_silgen_name("MirEngineViewportHover") public func MirEngineViewportHover(_ viewport: UnsafeMutableRawPointer?, _ x: Float, _ y: Float)
@_silgen_name("MirEngineViewportHoverClear") public func MirEngineViewportHoverClear(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineDeleteSelectedObject") public func MirEngineDeleteSelectedObject(_ viewport: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineClearSelection") public func MirEngineClearSelection(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineViewportDragCancel") public func MirEngineViewportDragCancel(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineUndo") public func MirEngineUndo(_ viewport: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineRedo") public func MirEngineRedo(_ viewport: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineCanUndo") public func MirEngineCanUndo(_ viewport: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineCanRedo") public func MirEngineCanRedo(_ viewport: UnsafeMutableRawPointer?) -> Bool
#endif

// MARK: - Live document bridge
#if !MIR4D_SWIFTPM
@_silgen_name("MIR4DDocumentCreate") public func MIR4DDocumentCreate() -> UnsafeMutableRawPointer?
@_silgen_name("MIR4DDocumentDestroy") public func MIR4DDocumentDestroy(_ handle: UnsafeMutableRawPointer?)
@_silgen_name("MIR4DDocumentReset") public func MIR4DDocumentReset(_ handle: UnsafeMutableRawPointer?, _ projectName: UnsafePointer<CChar>?)
@_silgen_name("MIR4DDocumentCreateBox") public func MIR4DDocumentCreateBox(_ handle: UnsafeMutableRawPointer?, _ width: Double, _ depth: Double, _ height: Double, _ objectId: UnsafeMutablePointer<UInt64>?) -> Bool
@_silgen_name("MIR4DDocumentAdvanceTime") public func MIR4DDocumentAdvanceTime(_ handle: UnsafeMutableRawPointer?, _ seconds: Double) -> Bool
@_silgen_name("MIR4DDocumentObjectCount") public func MIR4DDocumentObjectCount(_ handle: UnsafeMutableRawPointer?) -> Int
@_silgen_name("MIR4DDocumentCommandCount") public func MIR4DDocumentCommandCount(_ handle: UnsafeMutableRawPointer?) -> Int
@_silgen_name("MIR4DDocumentCurrentTime") public func MIR4DDocumentCurrentTime(_ handle: UnsafeMutableRawPointer?) -> Double
@_silgen_name("MIR4DDocumentRevision") public func MIR4DDocumentRevision(_ handle: UnsafeMutableRawPointer?) -> UInt64
@_silgen_name("MIR4DDocumentIsModified") public func MIR4DDocumentIsModified(_ handle: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MIR4DDocumentIsValid") public func MIR4DDocumentIsValid(_ handle: UnsafeMutableRawPointer?) -> Bool
#endif

public func cStrToString(_ pointer: UnsafePointer<CChar>?) -> String? {
    guard let pointer else { return nil }
    let value = String(cString: pointer)
    return value.isEmpty ? nil : value
}
public func cStrToStringDefault(_ pointer: UnsafePointer<CChar>?, _ defaultValue: String = "") -> String { cStrToString(pointer) ?? defaultValue }

public extension Color {
    init(hex: String) {
        var normalized = hex.trimmingCharacters(in: .whitespacesAndNewlines)
        if normalized.hasPrefix("#") { normalized.removeFirst() }
        var value: UInt64 = 0
        guard (normalized.count == 6 || normalized.count == 8), Scanner(string: normalized).scanHexInt64(&value) else { self = .black; return }
        let red = normalized.count == 6 ? Int((value >> 16) & 0xFF) : Int((value >> 24) & 0xFF)
        let green = normalized.count == 6 ? Int((value >> 8) & 0xFF) : Int((value >> 16) & 0xFF)
        let blue = normalized.count == 6 ? Int(value & 0xFF) : Int((value >> 8) & 0xFF)
        let alpha = normalized.count == 6 ? 255 : Int(value & 0xFF)
        self.init(.sRGB, red: Double(red) / 255.0, green: Double(green) / 255.0, blue: Double(blue) / 255.0, opacity: Double(alpha) / 255.0)
    }
}
