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


// ------------------------------------------------------------
// Selection
// ------------------------------------------------------------

uint64_t MirEngineGetSelectedObjectId(
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
// Error handling
// ------------------------------------------------------------

const char* MirEngineGetLastError(
    void* viewport
);

#ifdef __cplusplus
}
#endif
