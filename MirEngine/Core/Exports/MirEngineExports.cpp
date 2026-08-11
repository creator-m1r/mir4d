#include "MirEngineExports.h"

#include "../Rendering/OpenGL/OpenGLContext.h"
#include "../Rendering/OpenGL/OpenGLRenderer.h"
#include "../Platform/macOS/OpenGL/MacOpenGLContext.h"

using MirEngine::Rendering::OpenGLContext;
using MirEngine::Rendering::OpenGLRenderer;

extern "C"
{

void*
MirEngineCreateMacOpenGLContext(
    void* view,
    MirEngineSize2D size)
{
    auto* context =
        new MirEngine::Platform::macOS::MacOpenGLContext();

    MirEngine::Rendering::Size2D s{
        size.width,
        size.height
    };

    if (!context->initialize(view, s)) {
        delete context;

        return nullptr;
    }

    return context;
}

void
MirEngineDestroyOpenGLContext(
    void* context)
{
    delete static_cast<OpenGLContext*>(context);
}

void*
MirEngineCreateOpenGLRenderer(
    void* context)
{
    if (!context) {
        return nullptr;
    }

    return new OpenGLRenderer(
        static_cast<OpenGLContext*>(context)
    );
}

bool
MirEngineInitializeRenderer(
    void* renderer)
{
    if (!renderer) {
        return false;
    }

    return static_cast<OpenGLRenderer*>(
        renderer
    )->initialize();
}

void
MirEngineDestroyRenderer(
    void* renderer)
{
    delete static_cast<OpenGLRenderer*>(
        renderer
    );
}

void
MirEngineRender(
    void* renderer)
{
    if (!renderer) {
        return;
    }

    static_cast<OpenGLRenderer*>(
        renderer
    )->render();
}

void
MirEngineResize(
    void* renderer,
    uint32_t width,
    uint32_t height)
{
    if (!renderer) {
        return;
    }

    static_cast<OpenGLRenderer*>(
        renderer
    )->resize(
        width,
        height
    );
}

}