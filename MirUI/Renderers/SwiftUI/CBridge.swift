import Foundation
import SwiftUI

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

public struct MirTransform {
    public var px: Double
    public var py: Double
    public var pz: Double
    public var qx: Double
    public var qy: Double
    public var qz: Double
    public var qw: Double
    public var sx: Double
    public var sy: Double
    public var sz: Double
    public init() { px = 0; py = 0; pz = 0; qx = 0; qy = 0; qz = 0; qw = 1; sx = 1; sy = 1; sz = 1 }
    public init(px: Double = 0, py: Double = 0, pz: Double = 0,
                qx: Double = 0, qy: Double = 0, qz: Double = 0, qw: Double = 1,
                sx: Double = 1, sy: Double = 1, sz: Double = 1) {
        self.px = px; self.py = py; self.pz = pz
        self.qx = qx; self.qy = qy; self.qz = qz; self.qw = qw
        self.sx = sx; self.sy = sy; self.sz = sz
    }
}

#if MIR4D_SWIFTPM
public struct MirEngineSize2D {
    public var width: UInt32
    public var height: UInt32
    public init(width: UInt32, height: UInt32) { self.width = width; self.height = height }
}
public func MirEngineCreateMacOpenGLContext(_ view: UnsafeMutableRawPointer?, _ size: MirEngineSize2D) -> UnsafeMutableRawPointer? { nil }
public func MirEngineDestroyOpenGLContext(_ context: UnsafeMutableRawPointer?) {}
public func MirEngineSetOpenGLContextView(_ context: UnsafeMutableRawPointer?, _ view: UnsafeMutableRawPointer?) {}
public func MirEngineCreateOpenGLRenderer(_ context: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer? { nil }
public func MirEngineInitializeRenderer(_ renderer: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineDestroyRenderer(_ renderer: UnsafeMutableRawPointer?) {}
public func MirEngineCreateViewport(_ renderer: UnsafeMutableRawPointer?, _ width: UInt32, _ height: UInt32) -> UnsafeMutableRawPointer? { nil }
public func MirEngineDestroyViewport(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineRender(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineResize(_ viewport: UnsafeMutableRawPointer?, _ width: UInt32, _ height: UInt32) {}
public func MirEngineCreateBox(_ viewport: UnsafeMutableRawPointer?, _ width: Double, _ depth: Double, _ height: Double, _ objectId: UnsafeMutablePointer<UInt64>?) -> Bool { false }
public func MirEngineGetSelectedObjectMetrics(_ viewport: UnsafeMutableRawPointer?, _ outJson: UnsafeMutablePointer<CChar>?, _ outCapacity: Int) -> Bool { false }
public func MirEngineImportMesh(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?) -> Bool { false }
public func MirEngineGetLastError(_ viewport: UnsafeMutableRawPointer?) -> UnsafePointer<CChar>? { nil }
public func MirEngineExportStl(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?, _ selectionOnly: Bool) -> Bool { false }
public func MirEngineImportStep(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?) -> Bool { false }
public func MirEngineImportStepBRep(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?) -> Bool { false }
public func MirEngineExportStep(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?, _ selectionOnly: Bool) -> Bool { false }
public func MirEngineExportStepBRep(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?, _ selectionOnly: Bool) -> Bool { false }
public func MirEngineGetSelectedObjectId(_ viewport: UnsafeMutableRawPointer?) -> UInt64 { 0 }
public func MirEngineGetCameraOrientation(_ viewport: UnsafeMutableRawPointer?, _ theta: UnsafeMutablePointer<Float>?, _ phi: UnsafeMutablePointer<Float>?, _ distance: UnsafeMutablePointer<Float>?) {}
public func MirEngineSetCameraOrientation(_ viewport: UnsafeMutableRawPointer?, _ theta: Float, _ phi: Float, _ distance: Float) {}
public func MirEngineSetCameraProjection(_ viewport: UnsafeMutableRawPointer?, _ projection: Int32) {}
public func MirEngineGetCameraProjection(_ viewport: UnsafeMutableRawPointer?) -> Int32 { 0 }
public func MirEngineSetCameraFov(_ viewport: UnsafeMutableRawPointer?, _ fovYRadians: Float) {}
public func MirEngineSetActiveCameraPreset(_ viewport: UnsafeMutableRawPointer?, _ preset: Int32) {}
public func MirEngineGetCameraPresetOrientation(_ preset: Int32, _ theta: UnsafeMutablePointer<Float>?, _ phi: UnsafeMutablePointer<Float>?, _ distance: UnsafeMutablePointer<Float>?) {}
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
public func MirEngineSetCursor(_ renderer: UnsafeMutableRawPointer?, _ ndcX: Float, _ ndcY: Float, _ active: Bool) {}
public func MirEngineDeleteSelectedObject(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func mirEngineDeformSelected(_ viewport: UnsafeMutableRawPointer?, _ x: Double, _ y: Double, _ z: Double, _ radius: Double, _ strength: Double, _ mode: Int32) -> Bool { false }
public func mirEngineBeginDeformSelected(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func mirEngineEndDeformSelected(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func mirEngineSelectObject(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64) { }
public func mirEnginePickWorldPoint(_ viewport: UnsafeMutableRawPointer?, _ nx: Double, _ ny: Double, _ outX: UnsafeMutablePointer<Double>?, _ outY: UnsafeMutablePointer<Double>?, _ outZ: UnsafeMutablePointer<Double>?, _ outObjectId: UnsafeMutablePointer<UInt64>?) -> Bool { false }
public func MirEngineClearSelection(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineViewportDragCancel(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineUndo(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineRedo(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineCanUndo(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineCanRedo(_ viewport: UnsafeMutableRawPointer?) -> Bool { false }

public func MirEnginePickHandRay(_ viewport: UnsafeMutableRawPointer?, _ ox: Double, _ oy: Double, _ oz: Double, _ dx: Double, _ dy: Double, _ dz: Double, _ outObjectId: UnsafeMutablePointer<UInt64>?, _ outDistance: UnsafeMutablePointer<Double>?) -> Bool { false }
public func MirEngineBeginGrab(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64) {}
public func MirEnginePreviewGrab(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64, _ transform: MirTransform) -> Bool { false }
public func MirEngineCommitGrab(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64) -> Bool { false }
public func MirEngineCancelGrab(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineGetObjectTransform(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64, _ outTransform: UnsafeMutablePointer<MirTransform>?) -> Bool { false }
public func MirEngineSetHandHover(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64) {}

public func MirEngineSetHandSkeleton(_ viewport: UnsafeMutableRawPointer?, _ mode: Int32, _ handCount: Int32, _ positions: UnsafePointer<Double>?, _ confidence: UnsafePointer<Double>?, _ handedness: UnsafePointer<Int32>?, _ pinch: UnsafePointer<Double>?, _ gesture: UnsafePointer<Int32>?) {}
public func MirEngineSetHandSkeletonStyle(_ viewport: UnsafeMutableRawPointer?, _ leftR: Float, _ leftG: Float, _ leftB: Float, _ rightR: Float, _ rightG: Float, _ rightB: Float, _ jointSize: Float, _ tipSize: Float, _ wristSize: Float, _ alpha: Float, _ depthTest: Int32) {}
public func MirEngineSetHandSkeletonTopology(_ viewport: UnsafeMutableRawPointer?, _ boneCount: Int32, _ bones: UnsafePointer<Int32>?) {}
public func MirEngineClearHandSkeleton(_ viewport: UnsafeMutableRawPointer?) {}
public func MirEngineGetCameraEye(_ viewport: UnsafeMutableRawPointer?, _ outX: UnsafeMutablePointer<Double>?, _ outY: UnsafeMutablePointer<Double>?, _ outZ: UnsafeMutablePointer<Double>?) -> Bool { false }

public struct MirEnginePlane {
    public var id: UInt32
    public var origin: (Float, Float, Float)
    public var normal: (Float, Float, Float)
    public var xAxis: (Float, Float, Float)
    public var yAxis: (Float, Float, Float)
    public var color: (Float, Float, Float)
    public var size: Float
    public var active: Bool
    public var selected: Bool
    public init(id: UInt32, origin: (Float, Float, Float), normal: (Float, Float, Float),
                xAxis: (Float, Float, Float), yAxis: (Float, Float, Float),
                color: (Float, Float, Float), size: Float, active: Bool, selected: Bool) {
        self.id = id; self.origin = origin; self.normal = normal
        self.xAxis = xAxis; self.yAxis = yAxis; self.color = color
        self.size = size; self.active = active; self.selected = selected
    }
}
public func MirEnginePushWorkPlanes(_ renderer: UnsafeMutableRawPointer?, _ planes: [MirEnginePlane]) {}

public struct MirEngineSketchSegment: Sendable {
    public var ax: Float
    public var ay: Float
    public var bx: Float
    public var by: Float
    public var color: (Float, Float, Float)
    public init(ax: Float, ay: Float, bx: Float, by: Float, color: (Float, Float, Float)) {
        self.ax = ax; self.ay = ay; self.bx = bx; self.by = by; self.color = color
    }
}
public func MirEnginePushSketch(
    _ renderer: UnsafeMutableRawPointer?,
    _ segments: [MirEngineSketchSegment],
    origin: [Float] = [0, 0, 0],
    xAxis: [Float] = [1, 0, 0],
    yAxis: [Float] = [0, 1, 0]
) {}
public func MirEngineCreatePlaneStore() -> UnsafeMutableRawPointer? { nil }
public func MirEngineDestroyPlaneStore(_ store: UnsafeMutableRawPointer?) {}
public func MirEnginePlaneStoreAddBasePlanes(_ store: UnsafeMutableRawPointer?) {}
public func MirEnginePlaneStoreCreateOffsetPlane(_ store: UnsafeMutableRawPointer?, _ basePlane: UInt32, _ offset: Double, _ angleDeg: Double) -> UInt32 { 0 }
public func MirEnginePlaneStoreSnapshot(_ store: UnsafeMutableRawPointer?, _ maxCount: Int32, _ ids: UnsafeMutablePointer<UInt32>?, _ origins: UnsafeMutablePointer<Float>?, _ normals: UnsafeMutablePointer<Float>?, _ xAxes: UnsafeMutablePointer<Float>?, _ yAxes: UnsafeMutablePointer<Float>?, _ colors: UnsafeMutablePointer<Float>?, _ sizes: UnsafeMutablePointer<Float>?, _ active: UnsafeMutablePointer<Bool>?, _ selected: UnsafeMutablePointer<Bool>?) -> Int32 { 0 }
public func MirEngineSketchCreateDocument() -> UnsafeMutableRawPointer? { nil }
public func MirEngineSketchDestroyDocument(_ doc: UnsafeMutableRawPointer?) {}
public func MirEngineSketchAddLine(_ doc: UnsafeMutableRawPointer?, _ x1: Float, _ y1: Float, _ x2: Float, _ y2: Float) -> UInt32 { 0 }
public func MirEngineSketchAddCircle(_ doc: UnsafeMutableRawPointer?, _ cx: Float, _ cy: Float, _ r: Float) -> UInt32 { 0 }
public func MirEngineSketchAddConstraint(_ doc: UnsafeMutableRawPointer?, _ type: Int32, _ g1: UInt32, _ g2: UInt32, _ value: Double) -> UInt32 { 0 }
public func MirEngineSketchSolve(_ doc: UnsafeMutableRawPointer?) -> Bool { false }
public func MirEngineSketchGetLine(_ doc: UnsafeMutableRawPointer?, _ id: UInt32, _ x1: UnsafeMutablePointer<Float>?, _ y1: UnsafeMutablePointer<Float>?, _ x2: UnsafeMutablePointer<Float>?, _ y2: UnsafeMutablePointer<Float>?) -> Bool { false }
public func MirEngineSketchGetCircle(_ doc: UnsafeMutableRawPointer?, _ id: UInt32, _ cx: UnsafeMutablePointer<Float>?, _ cy: UnsafeMutablePointer<Float>?, _ r: UnsafeMutablePointer<Float>?) -> Bool { false }
public func MirEngineSketchAddArc(_ doc: UnsafeMutableRawPointer?, _ cx: Float, _ cy: Float, _ r: Float, _ startAngle: Float, _ endAngle: Float) -> UInt32 { 0 }
public func MirEngineSketchGeometryCount(_ doc: UnsafeMutableRawPointer?) -> UInt32 { 0 }
public func MirEngineSketchGeometryTypeAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32) -> Int32 { -1 }
public func MirEngineSketchLineAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ x1: UnsafeMutablePointer<Float>?, _ y1: UnsafeMutablePointer<Float>?, _ x2: UnsafeMutablePointer<Float>?, _ y2: UnsafeMutablePointer<Float>?) -> Bool { false }
public func MirEngineSketchArcAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ cx: UnsafeMutablePointer<Float>?, _ cy: UnsafeMutablePointer<Float>?, _ r: UnsafeMutablePointer<Float>?, _ sa: UnsafeMutablePointer<Float>?, _ ea: UnsafeMutablePointer<Float>?) -> Bool { false }
public func MirEngineSketchAddSpline(_ doc: UnsafeMutableRawPointer?, _ xs: UnsafePointer<Float>?, _ ys: UnsafePointer<Float>?, _ count: UInt32, _ closed: Bool) -> UInt32 { 0 }
public func MirEngineSketchSplineAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ xs: UnsafeMutablePointer<Float>?, _ ys: UnsafeMutablePointer<Float>?, _ count: UnsafeMutablePointer<UInt32>?, _ closed: UnsafeMutablePointer<Bool>?) -> Bool { false }
public func MirEngineSketchCircleAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ cx: UnsafeMutablePointer<Float>?, _ cy: UnsafeMutablePointer<Float>?, _ r: UnsafeMutablePointer<Float>?) -> Bool { false }
public func MirEngineSketchRemoveGeometry(_ doc: UnsafeMutableRawPointer?, _ id: UInt32) -> Bool { false }
public func MirEngineSketchClear(_ doc: UnsafeMutableRawPointer?) {}
public func MirEngineSketchGeometryIdAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32) -> UInt32 { 0 }
public func MirEngineSketchSetPlane(_ doc: UnsafeMutableRawPointer?, _ planeId: UInt32, _ ox: Float, _ oy: Float, _ oz: Float, _ nx: Float, _ ny: Float, _ nz: Float, _ xx: Float, _ xy: Float, _ xz: Float, _ yx: Float, _ yy: Float, _ yz: Float) {}
public func MirEngineSketchRemoveConstraint(_ doc: UnsafeMutableRawPointer?, _ id: UInt32) -> Bool { false }
public func MirEngineSketchConstraintCount(_ doc: UnsafeMutableRawPointer?) -> UInt32 { 0 }
public func MirEngineSketchConstraintAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ type: UnsafeMutablePointer<Int32>?, _ g1: UnsafeMutablePointer<UInt32>?, _ g2: UnsafeMutablePointer<UInt32>?, _ value: UnsafeMutablePointer<Double>?) -> Bool { false }

public func MirEngineRunCAECampaign(_ definition: UnsafePointer<CChar>?, _ outJson: UnsafeMutablePointer<CChar>?, _ outCapacity: Int) -> Bool { false }

#else
@_silgen_name("MirEngineCreateMacOpenGLContext") public func MirEngineCreateMacOpenGLContext(_ view: UnsafeMutableRawPointer?, _ size: MirEngineSize2D) -> UnsafeMutableRawPointer?
@_silgen_name("MirEngineDestroyOpenGLContext") public func MirEngineDestroyOpenGLContext(_ context: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineSetOpenGLContextView") public func MirEngineSetOpenGLContextView(_ context: UnsafeMutableRawPointer?, _ view: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineCreateOpenGLRenderer") public func MirEngineCreateOpenGLRenderer(_ context: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer?
@_silgen_name("MirEngineInitializeRenderer") public func MirEngineInitializeRenderer(_ renderer: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineDestroyRenderer") public func MirEngineDestroyRenderer(_ renderer: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineCreateViewport") public func MirEngineCreateViewport(_ renderer: UnsafeMutableRawPointer?, _ width: UInt32, _ height: UInt32) -> UnsafeMutableRawPointer?
@_silgen_name("MirEngineDestroyViewport") public func MirEngineDestroyViewport(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineRender") public func MirEngineRender(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineResize") public func MirEngineResize(_ viewport: UnsafeMutableRawPointer?, _ width: UInt32, _ height: UInt32)
@_silgen_name("MirEngineCreateBox") public func MirEngineCreateBox(_ viewport: UnsafeMutableRawPointer?, _ width: Double, _ depth: Double, _ height: Double, _ objectId: UnsafeMutablePointer<UInt64>?) -> Bool
@_silgen_name("MirEngineGetSelectedObjectMetrics") public func MirEngineGetSelectedObjectMetrics(_ viewport: UnsafeMutableRawPointer?, _ outJson: UnsafeMutablePointer<CChar>?, _ outCapacity: Int) -> Bool
@_silgen_name("MirEngineImportMesh") public func MirEngineImportMesh(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?) -> Bool
@_silgen_name("MirEngineGetLastError") public func MirEngineGetLastError(_ viewport: UnsafeMutableRawPointer?) -> UnsafePointer<CChar>?
@_silgen_name("MirEngineExportStl") public func MirEngineExportStl(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?, _ selectionOnly: Bool) -> Bool
@_silgen_name("MirEngineImportStep") public func MirEngineImportStep(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?) -> Bool
@_silgen_name("MirEngineImportStepBRep") public func MirEngineImportStepBRep(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?) -> Bool
@_silgen_name("MirEngineExportStep") public func MirEngineExportStep(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?, _ selectionOnly: Bool) -> Bool
@_silgen_name("MirEngineExportStepBRep") public func MirEngineExportStepBRep(_ viewport: UnsafeMutableRawPointer?, _ path: UnsafePointer<CChar>?, _ selectionOnly: Bool) -> Bool
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
@_silgen_name("MirEngineGetCameraPresetOrientation") public func MirEngineGetCameraPresetOrientation(_ preset: Int32, _ theta: UnsafeMutablePointer<Float>?, _ phi: UnsafeMutablePointer<Float>?, _ distance: UnsafeMutablePointer<Float>?)
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
@_silgen_name("MirEngineDeformSelected") public func mirEngineDeformSelected(_ viewport: UnsafeMutableRawPointer?, _ x: Double, _ y: Double, _ z: Double, _ radius: Double, _ strength: Double, _ mode: Int32) -> Bool
@_silgen_name("MirEngineBeginDeformSelected") public func mirEngineBeginDeformSelected(_ viewport: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineEndDeformSelected") public func mirEngineEndDeformSelected(_ viewport: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineClearSelection") public func MirEngineClearSelection(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineSelectObject") public func mirEngineSelectObject(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64)
@_silgen_name("MirEnginePickWorldPoint") public func mirEnginePickWorldPoint(_ viewport: UnsafeMutableRawPointer?, _ nx: Double, _ ny: Double, _ outX: UnsafeMutablePointer<Double>?, _ outY: UnsafeMutablePointer<Double>?, _ outZ: UnsafeMutablePointer<Double>?, _ outObjectId: UnsafeMutablePointer<UInt64>?) -> Bool
@_silgen_name("MirEngineViewportDragCancel") public func MirEngineViewportDragCancel(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineUndo") public func MirEngineUndo(_ viewport: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineRedo") public func MirEngineRedo(_ viewport: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineCanUndo") public func MirEngineCanUndo(_ viewport: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineCanRedo") public func MirEngineCanRedo(_ viewport: UnsafeMutableRawPointer?) -> Bool

@_silgen_name("MirEnginePickHandRay") public func MirEnginePickHandRay(_ viewport: UnsafeMutableRawPointer?, _ ox: Double, _ oy: Double, _ oz: Double, _ dx: Double, _ dy: Double, _ dz: Double, _ outObjectId: UnsafeMutablePointer<UInt64>?, _ outDistance: UnsafeMutablePointer<Double>?) -> Bool
@_silgen_name("MirEngineBeginGrab") public func MirEngineBeginGrab(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64)
@_silgen_name("MirEnginePreviewGrab") public func MirEnginePreviewGrab(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64, _ transform: MirTransform) -> Bool
@_silgen_name("MirEngineCommitGrab") public func MirEngineCommitGrab(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64) -> Bool
@_silgen_name("MirEngineCancelGrab") public func MirEngineCancelGrab(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineGetObjectTransform") public func MirEngineGetObjectTransform(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64, _ outTransform: UnsafeMutablePointer<MirTransform>?) -> Bool
@_silgen_name("MirEngineSetHandHover") public func MirEngineSetHandHover(_ viewport: UnsafeMutableRawPointer?, _ objectId: UInt64)
@_silgen_name("MirEngineSetHandSkeleton") public func MirEngineSetHandSkeleton(_ viewport: UnsafeMutableRawPointer?, _ mode: Int32, _ handCount: Int32, _ positions: UnsafePointer<Double>?, _ confidence: UnsafePointer<Double>?, _ handedness: UnsafePointer<Int32>?, _ pinch: UnsafePointer<Double>?, _ gesture: UnsafePointer<Int32>?)
@_silgen_name("MirEngineSetHandSkeletonStyle") public func MirEngineSetHandSkeletonStyle(_ viewport: UnsafeMutableRawPointer?, _ leftR: Float, _ leftG: Float, _ leftB: Float, _ rightR: Float, _ rightG: Float, _ rightB: Float, _ jointSize: Float, _ tipSize: Float, _ wristSize: Float, _ alpha: Float, _ depthTest: Int32)
@_silgen_name("MirEngineSetHandSkeletonTopology") public func MirEngineSetHandSkeletonTopology(_ viewport: UnsafeMutableRawPointer?, _ boneCount: Int32, _ bones: UnsafePointer<Int32>?)
@_silgen_name("MirEngineClearHandSkeleton") public func MirEngineClearHandSkeleton(_ viewport: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineGetCameraEye") public func MirEngineGetCameraEye(_ viewport: UnsafeMutableRawPointer?, _ outX: UnsafeMutablePointer<Double>?, _ outY: UnsafeMutablePointer<Double>?, _ outZ: UnsafeMutablePointer<Double>?) -> Bool
@_silgen_name("MirEngineSetPlanes") public func MirEngineSetPlanes(_ renderer: UnsafeMutableRawPointer?,
    _ count: Int32, _ ids: UnsafePointer<UInt32>?, _ origins: UnsafePointer<Float>?,
    _ normals: UnsafePointer<Float>?, _ xAxes: UnsafePointer<Float>?,
    _ yAxes: UnsafePointer<Float>?, _ colors: UnsafePointer<Float>?,
    _ sizes: UnsafePointer<Float>?, _ active: UnsafePointer<Bool>?, _ selected: UnsafePointer<Bool>?)

@_silgen_name("MirEngineSetCursor") public func MirEngineSetCursor(_ renderer: UnsafeMutableRawPointer?, _ ndcX: Float, _ ndcY: Float, _ active: Bool)

@_silgen_name("MirEngineSetSketch") public func MirEngineSetSketch(_ renderer: UnsafeMutableRawPointer?,
    _ segmentCount: Int32, _ ax: UnsafePointer<Float>?, _ ay: UnsafePointer<Float>?,
    _ bx: UnsafePointer<Float>?, _ by: UnsafePointer<Float>?,
    _ colors: UnsafePointer<Float>?, _ origin: UnsafePointer<Float>?,
    _ xAxis: UnsafePointer<Float>?, _ yAxis: UnsafePointer<Float>?)

public struct MirEnginePlane {
    public var id: UInt32
    public var origin: (Float, Float, Float)
    public var normal: (Float, Float, Float)
    public var xAxis: (Float, Float, Float)
    public var yAxis: (Float, Float, Float)
    public var color: (Float, Float, Float)
    public var size: Float
    public var active: Bool
    public var selected: Bool
    public init(id: UInt32, origin: (Float, Float, Float), normal: (Float, Float, Float),
                xAxis: (Float, Float, Float), yAxis: (Float, Float, Float),
                color: (Float, Float, Float), size: Float, active: Bool, selected: Bool) {
        self.id = id; self.origin = origin; self.normal = normal
        self.xAxis = xAxis; self.yAxis = yAxis; self.color = color
        self.size = size; self.active = active; self.selected = selected
    }
}

public func MirEnginePushWorkPlanes(_ renderer: UnsafeMutableRawPointer?, _ planes: [MirEnginePlane]) {
    guard let renderer, !planes.isEmpty else {
        if let renderer {
            MirEngineSetPlanes(renderer, 0, nil, nil, nil, nil, nil, nil, nil, nil, nil)
        }
        return
    }
    let count = planes.count
    var ids = [UInt32](); ids.reserveCapacity(count)
    var origins = [Float](); origins.reserveCapacity(count * 3)
    var normals = [Float](); normals.reserveCapacity(count * 3)
    var xAxes = [Float](); xAxes.reserveCapacity(count * 3)
    var yAxes = [Float](); yAxes.reserveCapacity(count * 3)
    var colors = [Float](); colors.reserveCapacity(count * 3)
    var sizes = [Float](); sizes.reserveCapacity(count)
    var active = [Bool](); active.reserveCapacity(count)
    var selected = [Bool](); selected.reserveCapacity(count)
    for p in planes {
        ids.append(p.id)
        origins.append(contentsOf: [p.origin.0, p.origin.1, p.origin.2])
        normals.append(contentsOf: [p.normal.0, p.normal.1, p.normal.2])
        xAxes.append(contentsOf: [p.xAxis.0, p.xAxis.1, p.xAxis.2])
        yAxes.append(contentsOf: [p.yAxis.0, p.yAxis.1, p.yAxis.2])
        colors.append(contentsOf: [p.color.0, p.color.1, p.color.2])
        sizes.append(p.size)
        active.append(p.active)
        selected.append(p.selected)
    }
    ids.withUnsafeBufferPointer { idsP in
        origins.withUnsafeBufferPointer { oP in
            normals.withUnsafeBufferPointer { nP in
                xAxes.withUnsafeBufferPointer { xP in
                    yAxes.withUnsafeBufferPointer { yP in
                        colors.withUnsafeBufferPointer { cP in
                            sizes.withUnsafeBufferPointer { sP in
                                active.withUnsafeBufferPointer { aP in
                                    selected.withUnsafeBufferPointer { sp in
                                        MirEngineSetPlanes(renderer, Int32(count),
                                            idsP.baseAddress, oP.baseAddress, nP.baseAddress,
                                            xP.baseAddress, yP.baseAddress, cP.baseAddress,
                                            sP.baseAddress, aP.baseAddress, sp.baseAddress)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

public struct MirEngineSketchSegment: Sendable {
    public var ax: Float
    public var ay: Float
    public var bx: Float
    public var by: Float
    public var color: (Float, Float, Float)
    public init(ax: Float, ay: Float, bx: Float, by: Float, color: (Float, Float, Float)) {
        self.ax = ax; self.ay = ay; self.bx = bx; self.by = by; self.color = color
    }
}

public func MirEnginePushSketch(
    _ renderer: UnsafeMutableRawPointer?,
    _ segments: [MirEngineSketchSegment],
    origin: [Float] = [0, 0, 0],
    xAxis: [Float] = [1, 0, 0],
    yAxis: [Float] = [0, 1, 0]
) {
    guard let renderer else {
        MirEngineSetSketch(nil, 0, nil, nil, nil, nil, nil, nil, nil, nil)
        return
    }
    let count = segments.count
    var ax = [Float](); ax.reserveCapacity(count)
    var ay = [Float](); ay.reserveCapacity(count)
    var bx = [Float](); bx.reserveCapacity(count)
    var by = [Float](); by.reserveCapacity(count)
    var colors = [Float](); colors.reserveCapacity(count * 3)
    for seg in segments {
        ax.append(seg.ax); ay.append(seg.ay); bx.append(seg.bx); by.append(seg.by)
        colors.append(contentsOf: [seg.color.0, seg.color.1, seg.color.2])
    }

    ax.withUnsafeBufferPointer { axP in
        ay.withUnsafeBufferPointer { ayP in
            bx.withUnsafeBufferPointer { bxP in
                by.withUnsafeBufferPointer { byP in
                    colors.withUnsafeBufferPointer { cP in
                        origin.withUnsafeBufferPointer { oP in
                            xAxis.withUnsafeBufferPointer { xP in
                                yAxis.withUnsafeBufferPointer { yP in
                                    MirEngineSetSketch(renderer, Int32(count),
                                        axP.baseAddress, ayP.baseAddress, bxP.baseAddress, byP.baseAddress,
                                        cP.baseAddress, oP.baseAddress, xP.baseAddress, yP.baseAddress)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

@_silgen_name("MirEngineCreatePlaneStore") public func MirEngineCreatePlaneStore() -> UnsafeMutableRawPointer?
@_silgen_name("MirEngineDestroyPlaneStore") public func MirEngineDestroyPlaneStore(_ store: UnsafeMutableRawPointer?)
@_silgen_name("MirEnginePlaneStoreAddBasePlanes") public func MirEnginePlaneStoreAddBasePlanes(_ store: UnsafeMutableRawPointer?)
@_silgen_name("MirEnginePlaneStoreCreateOffsetPlane") public func MirEnginePlaneStoreCreateOffsetPlane(_ store: UnsafeMutableRawPointer?, _ basePlane: UInt32, _ offset: Double, _ angleDeg: Double) -> UInt32
@_silgen_name("MirEnginePlaneStoreSnapshot") public func MirEnginePlaneStoreSnapshot(_ store: UnsafeMutableRawPointer?, _ maxCount: Int32, _ ids: UnsafeMutablePointer<UInt32>?, _ origins: UnsafeMutablePointer<Float>?, _ normals: UnsafeMutablePointer<Float>?, _ xAxes: UnsafeMutablePointer<Float>?, _ yAxes: UnsafeMutablePointer<Float>?, _ colors: UnsafeMutablePointer<Float>?, _ sizes: UnsafeMutablePointer<Float>?, _ active: UnsafeMutablePointer<Bool>?, _ selected: UnsafeMutablePointer<Bool>?) -> Int32
@_silgen_name("MirEngineSketchCreateDocument") public func MirEngineSketchCreateDocument() -> UnsafeMutableRawPointer?
@_silgen_name("MirEngineSketchDestroyDocument") public func MirEngineSketchDestroyDocument(_ doc: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineSketchAddLine") public func MirEngineSketchAddLine(_ doc: UnsafeMutableRawPointer?, _ x1: Float, _ y1: Float, _ x2: Float, _ y2: Float) -> UInt32
@_silgen_name("MirEngineSketchAddCircle") public func MirEngineSketchAddCircle(_ doc: UnsafeMutableRawPointer?, _ cx: Float, _ cy: Float, _ r: Float) -> UInt32
@_silgen_name("MirEngineSketchAddConstraint") public func MirEngineSketchAddConstraint(_ doc: UnsafeMutableRawPointer?, _ type: Int32, _ g1: UInt32, _ g2: UInt32, _ value: Double) -> UInt32
@_silgen_name("MirEngineSketchSolve") public func MirEngineSketchSolve(_ doc: UnsafeMutableRawPointer?) -> Bool
@_silgen_name("MirEngineSketchGetLine") public func MirEngineSketchGetLine(_ doc: UnsafeMutableRawPointer?, _ id: UInt32, _ x1: UnsafeMutablePointer<Float>?, _ y1: UnsafeMutablePointer<Float>?, _ x2: UnsafeMutablePointer<Float>?, _ y2: UnsafeMutablePointer<Float>?) -> Bool
@_silgen_name("MirEngineSketchGetCircle") public func MirEngineSketchGetCircle(_ doc: UnsafeMutableRawPointer?, _ id: UInt32, _ cx: UnsafeMutablePointer<Float>?, _ cy: UnsafeMutablePointer<Float>?, _ r: UnsafeMutablePointer<Float>?) -> Bool
@_silgen_name("MirEngineSketchAddArc") public func MirEngineSketchAddArc(_ doc: UnsafeMutableRawPointer?, _ cx: Float, _ cy: Float, _ r: Float, _ startAngle: Float, _ endAngle: Float) -> UInt32
@_silgen_name("MirEngineSketchGeometryCount") public func MirEngineSketchGeometryCount(_ doc: UnsafeMutableRawPointer?) -> UInt32
@_silgen_name("MirEngineSketchGeometryTypeAt") public func MirEngineSketchGeometryTypeAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32) -> Int32
@_silgen_name("MirEngineSketchLineAt") public func MirEngineSketchLineAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ x1: UnsafeMutablePointer<Float>?, _ y1: UnsafeMutablePointer<Float>?, _ x2: UnsafeMutablePointer<Float>?, _ y2: UnsafeMutablePointer<Float>?) -> Bool
@_silgen_name("MirEngineSketchArcAt") public func MirEngineSketchArcAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ cx: UnsafeMutablePointer<Float>?, _ cy: UnsafeMutablePointer<Float>?, _ r: UnsafeMutablePointer<Float>?, _ sa: UnsafeMutablePointer<Float>?, _ ea: UnsafeMutablePointer<Float>?) -> Bool
@_silgen_name("MirEngineSketchAddSpline") public func MirEngineSketchAddSpline(_ doc: UnsafeMutableRawPointer?, _ xs: UnsafePointer<Float>?, _ ys: UnsafePointer<Float>?, _ count: UInt32, _ closed: Bool) -> UInt32
@_silgen_name("MirEngineSketchSplineAt") public func MirEngineSketchSplineAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ xs: UnsafeMutablePointer<Float>?, _ ys: UnsafeMutablePointer<Float>?, _ count: UnsafeMutablePointer<UInt32>?, _ closed: UnsafeMutablePointer<Bool>?) -> Bool
@_silgen_name("MirEngineSketchCircleAt") public func MirEngineSketchCircleAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ cx: UnsafeMutablePointer<Float>?, _ cy: UnsafeMutablePointer<Float>?, _ r: UnsafeMutablePointer<Float>?) -> Bool
@_silgen_name("MirEngineSketchRemoveGeometry") public func MirEngineSketchRemoveGeometry(_ doc: UnsafeMutableRawPointer?, _ id: UInt32) -> Bool
@_silgen_name("MirEngineSketchClear") public func MirEngineSketchClear(_ doc: UnsafeMutableRawPointer?)
@_silgen_name("MirEngineSketchGeometryIdAt") public func MirEngineSketchGeometryIdAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32) -> UInt32
@_silgen_name("MirEngineSketchSetPlane") public func MirEngineSketchSetPlane(_ doc: UnsafeMutableRawPointer?, _ planeId: UInt32, _ ox: Float, _ oy: Float, _ oz: Float, _ nx: Float, _ ny: Float, _ nz: Float, _ xx: Float, _ xy: Float, _ xz: Float, _ yx: Float, _ yy: Float, _ yz: Float)
@_silgen_name("MirEngineSketchRemoveConstraint") public func MirEngineSketchRemoveConstraint(_ doc: UnsafeMutableRawPointer?, _ id: UInt32) -> Bool
@_silgen_name("MirEngineSketchConstraintCount") public func MirEngineSketchConstraintCount(_ doc: UnsafeMutableRawPointer?) -> UInt32
@_silgen_name("MirEngineSketchConstraintAt") public func MirEngineSketchConstraintAt(_ doc: UnsafeMutableRawPointer?, _ index: UInt32, _ type: UnsafeMutablePointer<Int32>?, _ g1: UnsafeMutablePointer<UInt32>?, _ g2: UnsafeMutablePointer<UInt32>?, _ value: UnsafeMutablePointer<Double>?) -> Bool

#endif

public enum MirEngineSketchConstraint: Int32 {
    case coincident = 0
    case horizontal
    case vertical
    case parallel
    case perpendicular
    case tangent
    case concentric
    case equal
    case symmetric
    case distance
    case angle
    case radius
    case diameter
}

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

@_silgen_name("MirEngineRunCAECampaign") public func MirEngineRunCAECampaign(_ definition: UnsafePointer<CChar>?, _ outJson: UnsafeMutablePointer<CChar>?, _ outCapacity: Int) -> Bool
#endif

public func MIR4DLog(_ category: String, _ message: String) {
    #if DEBUG
    print("[MIR4D-\(category)] \(message)")
    #endif
}

public func mirCString(_ pointer: UnsafePointer<CChar>?) -> String? {
    guard let pointer else { return nil }
    let utf8 = UnsafeRawPointer(pointer).assumingMemoryBound(to: UTF8.CodeUnit.self)
    return String(decodingCString: utf8, as: UTF8.self)
}

public func cStrToString(_ pointer: UnsafePointer<CChar>?) -> String? {
    guard let value = mirCString(pointer) else { return nil }
    return value.isEmpty ? nil : value
}
public func cStrToStringDefault(_ pointer: UnsafePointer<CChar>?, _ defaultValue: String = "") -> String { cStrToString(pointer) ?? defaultValue }

public func MIR4DRunCAECampaign(definition: String) -> String? {
    let capacity = 1 << 16
    let buffer = UnsafeMutablePointer<CChar>.allocate(capacity: capacity)
    defer { buffer.deallocate() }
    guard definition.withCString({ MirEngineRunCAECampaign($0, buffer, capacity) }) else { return nil }
    let utf8 = UnsafeRawPointer(buffer).assumingMemoryBound(to: UTF8.CodeUnit.self)
    return String(decodingCString: utf8, as: UTF8.self)
}

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
