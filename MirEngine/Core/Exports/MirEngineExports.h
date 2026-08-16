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

// Aborts an active object drag and restores the drag-start transform (Esc).
// No history entry is created.
void MirEngineViewportDragCancel(
    void* viewport
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

bool MirEngineImportMesh(
    void* viewport,
    const char* path
);

bool MirEngineExportStl(
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

// ------------------------------------------------------------
// Error handling
// ------------------------------------------------------------

const char* MirEngineGetLastError(
    void* viewport
);

#ifdef __cplusplus
}
#endif
