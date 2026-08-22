#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// MIR ENGINE — C ABI
// Swift / SwiftUI получает только этот API.
// Внутренние C++ классы наружу не выходят.
// ============================================================

typedef struct
{
    uint32_t width;
    uint32_t height;
} MirEngineSize2D;


// ------------------------------------------------------------
// OpenGL context
// ------------------------------------------------------------

void* MirEngineCreateMacOpenGLContext(
    void* view,
    MirEngineSize2D size
);

void MirEngineDestroyOpenGLContext(
    void* context
);

// Перепривязывает существующий OpenGL-контекст к новому NSView (remount).
// view == nullptr отвязывает контекст без его уничтожения.
void MirEngineSetOpenGLContextView(
    void* context,
    void* view
);


// ------------------------------------------------------------
// Renderer
// ------------------------------------------------------------

void* MirEngineCreateOpenGLRenderer(
    void* context
);

bool MirEngineInitializeRenderer(
    void* renderer
);

void MirEngineDestroyRenderer(
    void* renderer
);


// ------------------------------------------------------------
// Work planes (ТЗ Этап 1)
// ------------------------------------------------------------

/// Pushes the document's work planes to the renderer for viewport overlay.
/// Call when the plane set or selection changes (not per frame). Arrays are
/// flat: origins/normals/xAxes/yAxes/colors hold 3*count floats.
void MirEngineSetPlanes(
    void* renderer,
    int count,
    const uint32_t* ids,
    const float* origins,
    const float* normals,
    const float* xAxes,
    const float* yAxes,
    const float* colors,
    const float* sizes,
    const bool* active,
    const bool* selected
);

/// Sets the cursor position in normalized device coordinates for work-plane
/// hover picking. active=false clears the hover when the pointer leaves.
void MirEngineSetCursor(void* renderer, float ndcX, float ndcY, bool active);

/// Pushes a 2D sketch overlay (ТЗ Этап 2) drawn on a work plane. Segment
/// endpoints are in the plane's local frame. Pass count==0 to clear. Arrays:
///   ax,ay,bx,by : segmentCount floats each (local coords)
///   colors      : 3*segmentCount floats (rgb per segment)
///   origin/xAxis/yAxis : 3 floats each, the plane basis in world space
void MirEngineSetSketch(
    void* renderer,
    int segmentCount,
    const float* ax,
    const float* ay,
    const float* bx,
    const float* by,
    const float* colors,
    const float* origin,
    const float* xAxis,
    const float* yAxis
);

// ------------------------------------------------------------
// Work plane store (ТЗ Этап 1) — owns the document's planes.
// ------------------------------------------------------------

void* MirEngineCreatePlaneStore(void);
void MirEngineDestroyPlaneStore(void* store);
void MirEnginePlaneStoreAddBasePlanes(void* store);
uint32_t MirEnginePlaneStoreCreateOffsetPlane(void* store,
                                              uint32_t basePlane,
                                               double offset,
                                               double angleDeg);
void MirEnginePlaneStoreSetSelected(void* store, uint32_t id);
uint32_t MirEnginePickPlane(void* renderer, float ndcX, float ndcY);
int MirEnginePlaneStoreSnapshot(void* store,
                                int maxCount,
                                uint32_t* ids,
                                float* origins,
                                float* normals,
                                float* xAxes,
                                float* yAxes,
                                float* colors,
                                float* sizes,
                                bool* active,
                                bool* selected);


// ------------------------------------------------------------
// Viewport / Scene
// ------------------------------------------------------------

void* MirEngineCreateViewport(
    void* renderer,
    uint32_t width,
    uint32_t height
);

void MirEngineDestroyViewport(
    void* viewport
);

void MirEngineResize(
    void* viewport,
    uint32_t width,
    uint32_t height
);

void MirEngineRender(
    void* viewport
);


// ------------------------------------------------------------
// Camera
// ------------------------------------------------------------

void MirEngineGetCameraOrientation(
    void* viewport,
    float* theta,
    float* phi,
    float* distance
);

void MirEngineSetCameraOrientation(
    void* viewport,
    float theta,
    float phi,
    float distance
);

// Projection mode: 0 = perspective, 1 = orthographic.
void MirEngineSetCameraProjection(
    void* viewport,
    int projection
);

// Returns the active projection mode: 0 = perspective, 1 = orthographic.
int MirEngineGetCameraProjection(
    void* viewport
);

// Vertical field of view in radians (perspective mode only).
void MirEngineSetCameraFov(
    void* viewport,
    float fovYRadians
);

// Camera presets for the navigation sphere:
// 0 front, 1 back, 2 left, 3 right, 4 top, 5 bottom, 6 isometric.
void MirEngineSetActiveCameraPreset(
    void* viewport,
    int preset
);

// Returns the orbit angles (theta, phi, distance) of a camera preset
// without touching the viewport. Used to animate preset transitions.
void MirEngineGetCameraPresetOrientation(
    int preset,
    float* theta,
    float* phi,
    float* distance
);

void MirEngineFitViewport(
    void* viewport
);


// ------------------------------------------------------------
// Mouse / navigation
// ------------------------------------------------------------

void MirEngineViewportMouseDown(
    void* viewport,
    int button,
    float x,
    float y
);

void MirEngineViewportMouseUp(
    void* viewport,
    int button,
    float x,
    float y
);

void MirEngineViewportMouseMove(
    void* viewport,
    float x,
    float y
);

void MirEngineViewportScroll(
    void* viewport,
    float delta
);

// Zoom anchored at the cursor pixel (industrial zoom-to-cursor).
// Falls back to plain zoom when the picking ray is degenerate.
void MirEngineViewportZoomAt(
    void* viewport,
    float delta,
    float x,
    float y
);

void MirEngineViewportPan(
    void* viewport,
    float dx,
    float dy
);

void MirEngineViewportOrbit(
    void* viewport,
    float dx,
    float dy
);

void MirEngineViewportClick(
    void* viewport,
    float x,
    float y,
    bool addToSelection
);

// Updates the hover state from the cursor position. The hovered object
// receives a subtler highlight than the selection; hovering never changes
// the selection set.
void MirEngineViewportHover(
    void* viewport,
    float x,
    float y
);

// Clears the hover state (cursor left the viewport).
void MirEngineViewportHoverClear(
    void* viewport
);


// ------------------------------------------------------------
// Selection
// ------------------------------------------------------------

uint64_t MirEngineGetSelectedObjectId(
    void* viewport
);

// Sets the active pick/selection filter from a coarse mode.
// 0 = Body, 1 = Face, 2 = Edge, 3 = Vertex. The picker only returns the
// enabled kinds, enabling mode-aware sub-object selection.
void MirEngineSetSelectionFilter(
    void* viewport,
    int mode
);

// Returns the hierarchical kind of the primary selection:
// 0 = None, 1 = Vertex, 2 = Edge, 3 = Face, 4 = Body.
int MirEngineGetSelectionKind(
    void* viewport
);

// Returns the element id (vertex/edge index or B-Rep face id) of the primary
// selection. Valid when MirEngineGetSelectionKind reports a sub-object kind.
uint64_t MirEngineGetSelectionElementId(
    void* viewport
);

// Performs a rectangle (box) selection in screen space. `x0,y0,x1,y1` are
// pixel coordinates with a bottom-left origin. Every object whose projected
// bounding box intersects the rectangle becomes part of the multi selection;
// the primary selection is set to the first hit. When `additive` is true the
// new hits are unioned with the existing set instead of replacing it.
void MirEngineViewportBoxSelect(
    void* viewport,
    float x0,
    float y0,
    float x1,
    float y1,
    bool additive
);

// Returns the number of objects held by the current multi (box) selection.
int MirEngineGetSelectionCount(
    void* viewport
);

// Returns the object id at `index` within the multi selection, or 0 when the
// index is out of range.
uint64_t MirEngineGetSelectionItem(
    void* viewport,
    int index
);

// Returns the selection kind (PickKind: 0 None, 1 Body, 2 Face, 3 Edge,
// 4 Vertex) of the multi-selection entry at `index`, or 0 when out of range.
int MirEngineGetSelectionItemKind(
    void* viewport,
    int index
);

// Returns the element id (vertex index, edge id = ti*3 + k, or source B-Rep
// face id) of the multi-selection entry at `index`, or 0 when out of range.
uint64_t MirEngineGetSelectionItemElementId(
    void* viewport,
    int index
);

// Returns the geometric metric of an arbitrary element addressed by object id
// + kind + element id. `outArea` receives the summed face area (when kind is
// Face), `outLength` receives the chord length (when kind is Edge). Either
// output pointer may be NULL. Returns true when the element was found.
bool MirEngineGetElementMetric(
    void* viewport,
    uint64_t objectId,
    int kind,
    uint64_t elementId,
    double* outArea,
    double* outLength
);

// Applies an in-place sculpt/push/pull deformation to the selected object's
// tessellated mesh. `x,y,z` is the world-space brush centre, `radius` a
// world-space radius, `strength` a signed displacement magnitude in world
// units, and `mode` selects the displacement profile. Returns false when no
// object is selected or the selection has no editable mesh.
bool MirEngineDeformSelected(
    void* viewport,
    double x, double y, double z,
    double radius,
    double strength,
    int mode
);

// Begins an undoable sculpt stroke on the primary selection: snapshots the
// current mesh vertices. Pairs with MirEngineEndDeformSelected. No-op when
// nothing is selected.
bool MirEngineBeginDeformSelected(
    void* viewport
);

// Commits a single undoable DeformObjectCommand for the active stroke (only if
// the mesh actually changed since MirEngineBeginDeformSelected). Pairs with
// MirEngineBeginDeformSelected.
bool MirEngineEndDeformSelected(
    void* viewport
);

// Deletes the primary selection through the canonical Scene API. The
// renderer observes the scene change; nothing is removed from the renderer
// directly. Returns true when an object was removed.
bool MirEngineDeleteSelectedObject(
    void* viewport
);

// Clears the selection set without modifying the scene.
void MirEngineClearSelection(
    void* viewport
);

// Selects the object with the given engine id in the viewport without a pick.
// Keeps the engineering selection in sync with the CAD/app selection so tools
// that read viewport selection (e.g. sculpt) work regardless of how the body
// was picked (3D viewport click or CAD tree).
void MirEngineSelectObject(
    void* viewport,
    uint64_t objectId
);

// Ray-casts the viewport camera through normalized screen coords (nx, ny in
// -1..1, screen-centred, y up) and returns the world hit point on the closest
// mesh plus the hit object id. Used to place the air-sculpt brush exactly where
// the hand points, independent of camera orientation. Returns false on a miss.
bool MirEnginePickWorldPoint(
    void* viewport,
    double nx, double ny,
    double* outX, double* outY, double* outZ,
    uint64_t* outObjectId
);

// Aborts an active object drag and restores the drag-start transform (Esc).
// No history entry is created.
void MirEngineViewportDragCancel(
    void* viewport
);

// ------------------------------------------------------------
// Hand Grab — Vertical Slice v0.1 (Pinch → point → grab → move → commit)
// ------------------------------------------------------------

/// Canonical object transform passed across the C ABI (position / rotation /
/// scale). Mirrors mir4d::Transform in the engine.
typedef struct
{
    double px, py, pz;
    double qx, qy, qz, qw;
    double sx, sy, sz;
} MirTransform;

/// Picks the scene against an explicit world-space hand ray. Returns the hit
/// object id (0 on a miss) and the distance from the ray origin.
bool MirEnginePickHandRay(
    void* viewport,
    double ox, double oy, double oz,
    double dx, double dy, double dz,
    uint64_t* outObjectId,
    double* outDistance
);

/// Arms a grab on the given object, snapshots its transform and selects it.
void MirEngineBeginGrab(
    void* viewport,
    uint64_t objectId
);

/// Live preview of the grabbed object's transform. Mutates the scene with no
/// history entry (Preview, not Commit).
bool MirEnginePreviewGrab(
    void* viewport,
    uint64_t objectId,
    MirTransform transform
);

/// Commits exactly one undoable Move/Transform command for the active grab.
/// Returns false when no grab is active or nothing moved.
bool MirEngineCommitGrab(
    void* viewport,
    uint64_t objectId
);

/// Cancels the active grab and restores the snapshot transform (no history).
void MirEngineCancelGrab(
    void* viewport
);

/// Current world transform of an object (seed for preview deltas).
bool MirEngineGetObjectTransform(
    void* viewport,
    uint64_t objectId,
    MirTransform* outTransform
);

/// Устанавливает мировой transform объекта (позиция / поворот / масштаб).
/// Используется, например, для параметрической правки тела без пересоздания
/// геометрии (масштаб вдоль нормали плоскости = глубина выдавливания).
void MirEngineSetObjectTransform(
    void* viewport,
    uint64_t objectId,
    const MirTransform* transform
);

/// Строит мировой луч через пиксель экрана (для перетаскивания объектов).
/// origin/dir — массивы из 3 double (точка и направление луча).
void MirEngineViewportRay(
    void* viewport,
    float x,
    float y,
    double origin[3],
    double dir[3]
);

/// Highlights an object under the hand (hover) without changing selection.
void MirEngineSetHandHover(
    void* viewport,
    uint64_t objectId
);

/// Pushes the hand-skeleton overlay for the current frame (debug / assist).
/// `mode` mirrors MIRHandSkeletonVisMode (0 off, 1 joints, 2 bones, 3 bones+ray).
/// `positions` is handCount*21*3 doubles in LandmarkID.allCases order,
/// `confidence` handCount*21, `handedness` handCount (0 left, 1 right, 2 none),
/// `pinch` handCount, `gesture` handCount (index into MIRHandGestureType.allCases).
/// A null/empty call clears the overlay.
void MirEngineSetHandSkeleton(
    void* viewport,
    int32_t mode,
    int32_t handCount,
    const double* positions,
    const double* confidence,
    const int32_t* handedness,
    const double* pinch,
    const int32_t* gesture
);

/// Sets the hand-skeleton overlay style (colours, sizes, transparency, depth).
void MirEngineSetHandSkeletonStyle(
    void* viewport,
    float leftR, float leftG, float leftB,
    float rightR, float rightG, float rightB,
    float jointSize, float tipSize, float wristSize,
    float alpha, int32_t depthTest
);

/// Sets the hand-skeleton bone topology. `bones` is boneCount pairs of indices
/// (parent, child) into the 21-joint array ordered by LandmarkID.allCases.
void MirEngineSetHandSkeletonTopology(
    void* viewport,
    int32_t boneCount,
    const int32_t* bones
);

/// Clears the hand-skeleton overlay.
void MirEngineClearHandSkeleton(void* viewport);

/// World-space camera eye used to build the hand picking ray. Returns false
/// when the viewport / camera is not ready; outputs are left untouched then.
bool MirEngineGetCameraEye(
    void* viewport,
    double* outX,
    double* outY,
    double* outZ
);

// Undo / Redo of scene commands (Move / Delete). Returns true when a
// command was reverted / reapplied.
bool MirEngineUndo(
    void* viewport
);

bool MirEngineRedo(
    void* viewport
);

bool MirEngineCanUndo(
    void* viewport
);

bool MirEngineCanRedo(
    void* viewport
);


// ------------------------------------------------------------
// Geometry
// ------------------------------------------------------------

bool MirEngineCreateBox(
    void* viewport,
    double width,
    double depth,
    double height,
    uint64_t* objectId
);

/// Выдавливает прямоугольный профиль эскиза в твёрдое тело на рабочей плоскости.
/// (width, height) — размеры профиля; (cu, cv) — центр профиля в локальных
/// координатах плоскости (U,V); distance — глубина выдавливания;
/// (ox,oy,oz) — origin плоскости; (nx,ny,nz) — нормаль; (xx,xy,xz)/(yx,yy,yz) —
/// оси X/Y плоскости. Возвращает id созданного объекта (0 при неудаче).
uint64_t MirEngineExtrudeSketch(void* viewport,
                                double width,
                                double height,
                                double cu,
                                double cv,
                                double distance,
                                double ox, double oy, double oz,
                                double nx, double ny, double nz,
                                double xx, double xy, double xz,
                                double yx, double yy, double yz);

/// Выдавливает произвольный замкнутый контур эскиза в твёрдое тело.
/// uv — плоский массив [u0,v0,u1,v1,...] (локальные координаты плоскости,
/// совпадающие с наброском); count — число точек; distance — глубина;
/// остальные параметры задают плоскость (см. MirEngineExtrudeSketch).
/// Возвращает id созданного объекта (0 при неудаче).
uint64_t MirEngineExtrudeContour(void* viewport,
                                 const double* uv,
                                 int count,
                                 double distance,
                                 double ox, double oy, double oz,
                                 double nx, double ny, double nz,
                                 double xx, double xy, double xz,
                                 double yx, double yy, double yz);

// Extracts real geometry metrics (bounding box, size, volume, surface area,
// vertex/face counts) from the primary selected object in the viewport scene.
// Writes a JSON document into outJson (capacity outCapacity) and returns true
// when the buffer was written. When nothing is selected or the object has no
// tessellated mesh, writes {"hasGeometry":false}.
bool MirEngineGetSelectedObjectMetrics(
    void* viewport,
    char* outJson,
    size_t outCapacity
);

// Same JSON geometry brief as MirEngineGetSelectedObjectMetrics, but for an
// arbitrary object addressed by id. Used by the multi-selection inspector to
// show per-item metrics (bounding box, volume, surface area, mesh counts).
bool MirEngineGetObjectMetricsById(
    void* viewport,
    uint64_t objectId,
    char* outJson,
    size_t outCapacity
);

// Returns the world-space surface area (in scene units) of the currently
// selected face. Returns 0.0 when the selection is not a face, the element has
// no B-Rep face provenance, or nothing is selected.
double MirEngineGetSelectionFaceArea(void* viewport);

// Returns the world-space length (in scene units) of the currently selected
// edge. Returns 0.0 when the selection is not an edge or nothing is selected.
double MirEngineGetSelectionEdgeLength(void* viewport);

// Returns the stable source B-Rep edge id of the currently selected edge, or
// UINT64_MAX (mir::kInvalidSourceEdge) when the selection is not an edge or has
// no B-Rep provenance. The id mirrors BRepEdgeHandle::index.
uint64_t MirEngineGetSelectionEdgeSourceId(void* viewport);

bool MirEngineImportMesh(
    void* viewport,
    const char* path
);

bool MirEngineExportStl(
    void* viewport,
    const char* path,
    bool selectionOnly
);

// Opens a STEP (.step / .stp) model and inserts its tessellated geometry
// into the viewport scene as a render mesh.
bool MirEngineImportStep(
    void* viewport,
    const char* path
);

// Opens a STEP (.step / .stp) model and inserts its exact B-Rep geometry
// (native BRepStepBridge) tessellated into the viewport scene as a render mesh.
bool MirEngineImportStepBRep(
    void* viewport,
    const char* path
);

// Saves the exact B-Rep sources of the viewport scene (or the current
// selection) to a STEP model using the native BRepStepBridge writer.
bool MirEngineExportStepBRep(
    void* viewport,
    const char* path,
    bool selectionOnly
);

// Saves the viewport scene (or the current selection) as a STEP model using
// the native faceted_brep writer.
bool MirEngineExportStep(
    void* viewport,
    const char* path,
    bool selectionOnly
);


// ------------------------------------------------------------
// Materials (procedural MaterialLibrary, no textures)
// ------------------------------------------------------------

int32_t MirEngineMaterialCount(void);

bool MirEngineMaterialName(
    int32_t materialId,
    char* buffer,
    size_t bufferSize
);

bool MirEngineSetObjectMaterial(
    void* viewport,
    uint64_t objectId,
    int32_t materialId
);


// ------------------------------------------------------------
// OpenGL diagnostics
// ------------------------------------------------------------

bool MirEngineGetOpenGLDiagnostics(
    void* renderer,
    char* buffer,
    size_t bufferSize
);


// ------------------------------------------------------------
// Sketch solver (universal constraint solver)
// ------------------------------------------------------------

typedef struct MirEngineSketchDocument MirEngineSketchDocument;

typedef enum MirEngineSketchConstraint
{
    MirEngineSketchCoincident = 0,
    MirEngineSketchHorizontal,
    MirEngineSketchVertical,
    MirEngineSketchParallel,
    MirEngineSketchPerpendicular,
    MirEngineSketchTangent,
    MirEngineSketchConcentric,
    MirEngineSketchEqual,
    MirEngineSketchSymmetric,
    MirEngineSketchDistance,
    MirEngineSketchAngle,
    MirEngineSketchRadius,
    MirEngineSketchDiameter
} MirEngineSketchConstraint;

void* MirEngineSketchCreateDocument(void);
void MirEngineSketchDestroyDocument(void* doc);
uint32_t MirEngineSketchAddLine(void* doc, float x1, float y1, float x2, float y2);
uint32_t MirEngineSketchAddCircle(void* doc, float cx, float cy, float r);
uint32_t MirEngineSketchAddConstraint(void* doc, int type, uint32_t g1, uint32_t g2, double value);
bool MirEngineSketchSolve(void* doc);
bool MirEngineSketchGetLine(void* doc, uint32_t id, float* x1, float* y1, float* x2, float* y2);
bool MirEngineSketchGetCircle(void* doc, uint32_t id, float* cx, float* cy, float* r);
uint32_t MirEngineSketchAddArc(void* doc, float cx, float cy, float r, float startAngle, float endAngle);
uint32_t MirEngineSketchAddSpline(void* doc, const float* xs, const float* ys, uint32_t count, bool closed);
uint32_t MirEngineSketchSplineCount(void* doc);
bool MirEngineSketchSplineAt(void* doc, uint32_t index, float* xs, float* ys, uint32_t* count, bool* closed);
uint32_t MirEngineSketchGeometryCount(void* doc);
int MirEngineSketchGeometryTypeAt(void* doc, uint32_t index);
bool MirEngineSketchLineAt(void* doc, uint32_t index, float* x1, float* y1, float* x2, float* y2);
bool MirEngineSketchArcAt(void* doc, uint32_t index, float* cx, float* cy, float* r, float* sa, float* ea);
bool MirEngineSketchCircleAt(void* doc, uint32_t index, float* cx, float* cy, float* r);
bool MirEngineSketchRemoveGeometry(void* doc, uint32_t id);
void MirEngineSketchClear(void* doc);
uint32_t MirEngineSketchGeometryIdAt(void* doc, uint32_t index);
void MirEngineSketchSetPlane(void* doc, uint32_t planeId,
                             float ox, float oy, float oz,
                             float nx, float ny, float nz,
                             float xx, float xy, float xz,
                             float yx, float yy, float yz);
bool MirEngineSketchRemoveConstraint(void* doc, uint32_t id);
uint32_t MirEngineSketchConstraintCount(void* doc);
bool MirEngineSketchConstraintAt(void* doc, uint32_t index, int32_t* type, uint32_t* g1, uint32_t* g2, double* value);

// ------------------------------------------------------------
// Sketch Session — authoritative runtime facade for the editor
// ------------------------------------------------------------

typedef struct MirEngineSketchSession MirEngineSketchSession;

typedef enum MirEngineSketchSolverStatus
{
    MirEngineSketchSolverNotRun = 0,
    MirEngineSketchSolverSolved = 1,
    MirEngineSketchSolverUnderConstrained = 2,
    MirEngineSketchSolverOverConstrained = 3,
    MirEngineSketchSolverFailed = 4
} MirEngineSketchSolverStatus;

typedef enum MirEngineSketchGeometryType
{
    MirEngineSketchGeomLine = 0,
    MirEngineSketchGeomArc = 1,
    MirEngineSketchGeomCircle = 2,
    MirEngineSketchGeomSpline = 3
} MirEngineSketchGeometryType;

typedef struct MirEngineSketchSessionState
{
    int solverStatus;          // MirEngineSketchSolverStatus
    int degreesOfFreedom;
    bool canUndo;
    bool canRedo;
    uint64_t revision;
    uint64_t geometryCount;
    uint64_t constraintCount;
    uint64_t profileCount;
} MirEngineSketchSessionState;

MirEngineSketchSession* MirEngineSketchSessionCreate(void);
void MirEngineSketchSessionDestroy(MirEngineSketchSession* session);

uint32_t MirEngineSketchSessionCreateLine(MirEngineSketchSession* session, float x1, float y1, float x2, float y2);
uint32_t MirEngineSketchSessionCreateCircle(MirEngineSketchSession* session, float cx, float cy, float r);
uint32_t MirEngineSketchSessionCreateArc(MirEngineSketchSession* session, float cx, float cy, float r, float startAngle, float endAngle);
uint32_t MirEngineSketchSessionCreateRectangle(MirEngineSketchSession* session, float x1, float y1, float x2, float y2);
uint32_t MirEngineSketchSessionCreateSpline(MirEngineSketchSession* session, const float* xs, const float* ys, uint32_t count, bool closed);
bool MirEngineSketchSessionDeleteGeometry(MirEngineSketchSession* session, uint32_t id);

uint32_t MirEngineSketchSessionAddConstraint(MirEngineSketchSession* session, int type, uint32_t g1, uint32_t g2, double value);
bool MirEngineSketchSessionRemoveConstraint(MirEngineSketchSession* session, uint32_t id);

bool MirEngineSketchSessionSolve(MirEngineSketchSession* session);
void MirEngineSketchSessionGetState(MirEngineSketchSession* session, MirEngineSketchSessionState* out);

uint32_t MirEngineSketchSessionGeometryCount(MirEngineSketchSession* session);
int MirEngineSketchSessionGeometryTypeAt(MirEngineSketchSession* session, uint32_t index);
uint32_t MirEngineSketchSessionGeometryIdAt(MirEngineSketchSession* session, uint32_t index);
bool MirEngineSketchSessionLineAt(MirEngineSketchSession* session, uint32_t index, float* x1, float* y1, float* x2, float* y2);
bool MirEngineSketchSessionArcAt(MirEngineSketchSession* session, uint32_t index, float* cx, float* cy, float* r, float* sa, float* ea);
bool MirEngineSketchSessionCircleAt(MirEngineSketchSession* session, uint32_t index, float* cx, float* cy, float* r);
bool MirEngineSketchSessionSplineAt(MirEngineSketchSession* session, uint32_t index, float* xs, float* ys, uint32_t* count, bool* closed);
uint32_t MirEngineSketchSessionConstraintCount(MirEngineSketchSession* session);
bool MirEngineSketchSessionConstraintAt(MirEngineSketchSession* session, uint32_t index, int32_t* type, uint32_t* g1, uint32_t* g2, double* value);

void MirEngineSketchSessionSelect(MirEngineSketchSession* session, uint32_t id, bool additive);
void MirEngineSketchSessionClearSelection(MirEngineSketchSession* session);
uint32_t MirEngineSketchSessionSelectedCount(MirEngineSketchSession* session);
uint32_t MirEngineSketchSessionSelectedAt(MirEngineSketchSession* session, uint32_t index);

bool MirEngineSketchSessionUndo(MirEngineSketchSession* session);
bool MirEngineSketchSessionRedo(MirEngineSketchSession* session);

/// Строит замкнутый профиль из геометрии сессии (линии/дуги) и выдавливает
/// его в 3D-сцену, связанную с viewport. Возвращает id созданного объекта.
uint64_t MirEngineSketchSessionExtrude(
    MirEngineSketchSession* session,
    void* viewport,
    double distance,
    double ox, double oy, double oz,
    double nx, double ny, double nz,
    double xx, double xy, double xz,
    double yx, double yy, double yz);

// ------------------------------------------------------------
// VFX — собственная подсистема визуальных эффектов (MirEngine::VFX)
// ------------------------------------------------------------

/// kind: 0=Confetti 1=Balloons 2=Fireworks 3=Rain 4=Hearts 5=Lasers
void MirEngineVFXTrigger(int kind);
void MirEngineVFXUpdate(float dt);
void MirEngineVFXRender();
void MirEngineVFXReset();

// ------------------------------------------------------------
// Error handling
// ------------------------------------------------------------

const char* MirEngineGetLastError(
    void* viewport
);

#ifdef __cplusplus
}
#endif
