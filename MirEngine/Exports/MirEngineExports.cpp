#include "MirEngineExports.h"

#include "../../Platform/macOS/OpenGL/MacOpenGLContext.h"
#include "../../Rendering/OpenGL/OpenGLRenderer.h"
#include "../../Rendering/OpenGL/OpenGLContext.h"

using MirEngine::Platform::macOS::MacOpenGLContext;
using MirEngine::Rendering::OpenGLRenderer;
using MirEngine::Rendering::OpenGLContext;
using MirEngine::Rendering::Size2D;

extern "C" {

void* MirEngineCreateMacOpenGLContext(void* view, MirEngineSize2D size)
{
    auto* ctx = new MacOpenGLContext();
    Size2D s{ size.width, size.height };

    if (!ctx->initialize(view, s)) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

void MirEngineDestroyOpenGLContext(void* context)
{
    delete static_cast<OpenGLContext*>(context);
}

void* MirEngineCreateOpenGLRenderer(void* context)
{
    if (!context) return nullptr;
    return new OpenGLRenderer(static_cast<OpenGLContext*>(context));
}

bool MirEngineInitializeRenderer(void* renderer)
{
    if (!renderer) return false;
    return static_cast<OpenGLRenderer*>(renderer)->initialize();
}

void MirEngineDestroyRenderer(void* renderer)
{
    delete static_cast<OpenGLRenderer*>(renderer);
}

void MirEngineRender(void* renderer)
{
    if (!renderer) return;
    static_cast<OpenGLRenderer*>(renderer)->render();
}

void MirEngineResize(void* renderer, uint32_t width, uint32_t height)
{
    if (!renderer) return;
    static_cast<OpenGLRenderer*>(renderer)->resize(width, height);
}

} // extern "C"