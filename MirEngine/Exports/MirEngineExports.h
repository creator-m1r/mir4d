#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t width;
    uint32_t height;
} MirEngineSize2D;

void* MirEngineCreateMacOpenGLContext(void* view, MirEngineSize2D size);
void MirEngineDestroyOpenGLContext(void* context);
void* MirEngineCreateOpenGLRenderer(void* context);
bool MirEngineInitializeRenderer(void* renderer);
void MirEngineDestroyRenderer(void* renderer);
void* MirEngineCreateViewport(void* renderer, uint32_t width, uint32_t height);
void MirEngineDestroyViewport(void* viewport);
void MirEngineRender(void* viewport);
void MirEngineResize(void* viewport, uint32_t width, uint32_t height);

// Document/scene geometry commands. Geometry is created in MirEngine's
// OpenGL viewport Scene and is therefore rendered by the same renderer.
bool MirEngineCreateBox(
    void* viewport,
    double width,
    double depth,
    double height,
    uint64_t* objectId
);

void MirEngineViewportMouseDown(void* viewport, int button, float x, float y);
void MirEngineViewportMouseUp(void* viewport, int button, float x, float y);
void MirEngineViewportMouseMove(void* viewport, float x, float y);
void MirEngineViewportScroll(void* viewport, float delta);
void MirEngineViewportClick(void* viewport, float x, float y, bool addToSelection);

#ifdef __cplusplus
}
#endif
