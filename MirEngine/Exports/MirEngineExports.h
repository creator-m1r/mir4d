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

// Создание / уничтожение macOS OpenGL-контекста
void* MirEngineCreateMacOpenGLContext(void* view, MirEngineSize2D size);
void  MirEngineDestroyOpenGLContext(void* context);

// Создание / уничтожение рендерера
void* MirEngineCreateOpenGLRenderer(void* context);
bool  MirEngineInitializeRenderer(void* renderer);
void  MirEngineDestroyRenderer(void* renderer);

// Кадр и resize
void MirEngineRender(void* renderer);
void MirEngineResize(void* renderer, uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif