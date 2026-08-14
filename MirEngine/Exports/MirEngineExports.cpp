#include "MirEngineExports.h"

#include "../Platform/macOS/OpenGL/MacOpenGLContext.h"
#include "../Rendering/OpenGL/OpenGLRenderer.h"
#include "../Rendering/OpenGL/OpenGLContext.h"
#include "../Viewport/ViewportRuntime.hpp"
#include "../Geometry/Scene/Scene.hpp"
#include "../BRep/Core/BRepModel.hpp"
#include "../BRep/Commands/BRepSceneBridge.hpp"

#include <algorithm>
#include <memory>

using MirEngine::Platform::macOS::MacOpenGLContext;
using MirEngine::Rendering::OpenGLRenderer;
using MirEngine::Rendering::OpenGLContext;
using MirEngine::Rendering::Size2D;

namespace
{

struct NativeViewport
{
    std::unique_ptr<mir::ViewportRuntime> runtime;
    std::unique_ptr<mir::Scene> scene;
    std::unique_ptr<mir::BRepModel> brep;
};

NativeViewport* asViewport(void* handle) noexcept
{
    return static_cast<NativeViewport*>(handle);
}

} // namespace

extern "C"
{

void* MirEngineCreateMacOpenGLContext(void* view, MirEngineSize2D size)
{
    auto* context = new MacOpenGLContext();
    const Size2D nativeSize{size.width, size.height};

    if (!context->initialize(view, nativeSize))
    {
        delete context;
        return nullptr;
    }

    return context;
}

void MirEngineDestroyOpenGLContext(void* context)
{
    delete static_cast<OpenGLContext*>(context);
}

void* MirEngineCreateOpenGLRenderer(void* context)
{
    if (!context)
        return nullptr;

    return new OpenGLRenderer(static_cast<OpenGLContext*>(context));
}

bool MirEngineInitializeRenderer(void* renderer)
{
    if (!renderer)
        return false;

    return static_cast<OpenGLRenderer*>(renderer)->initialize();
}

void MirEngineDestroyRenderer(void* renderer)
{
    delete static_cast<OpenGLRenderer*>(renderer);
}

void* MirEngineCreateViewport(void* renderer, uint32_t width, uint32_t height)
{
    if (!renderer)
        return nullptr;

    auto native = std::make_unique<NativeViewport>();
    native->scene = std::make_unique<mir::Scene>();
    native->brep = std::make_unique<mir::BRepModel>();

    const uint32_t safeWidth = std::max(width, 1u);
    const uint32_t safeHeight = std::max(height, 1u);
    auto* nativeRenderer = static_cast<OpenGLRenderer*>(renderer);

    native->runtime = std::make_unique<mir::ViewportRuntime>(nativeRenderer);
    native->runtime->setScene(native->scene.get());
    native->runtime->resize(safeWidth, safeHeight);

    auto& camera = native->runtime->state().camera;
    camera.setPerspective(
        mir::Scalar(0.7853981633974483),
        static_cast<mir::Scalar>(safeWidth) / static_cast<mir::Scalar>(safeHeight),
        mir::Scalar(0.1),
        mir::Scalar(10000.0));

    return native.release();
}

void MirEngineDestroyViewport(void* viewport)
{
    delete asViewport(viewport);
}

void MirEngineResizeViewport(void* viewport, uint32_t width, uint32_t height)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    const uint32_t safeWidth = std::max(width, 1u);
    const uint32_t safeHeight = std::max(height, 1u);
    native->runtime->resize(safeWidth, safeHeight);
}

void MirEngineRenderViewport(void* viewport)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->render();
}

bool MirEngineCreateBox(void* viewport,
                        double width,
                        double depth,
                        double height,
                        uint64_t* objectId)
{
    auto* native = asViewport(viewport);
    if (!native || !native->scene || !native->brep)
        return false;

    const auto result = mir4d::BRepSceneBridge::createBox(
        *native->scene,
        *native->brep,
        static_cast<mir::Scalar>(width),
        static_cast<mir::Scalar>(depth),
        static_cast<mir::Scalar>(height));

    if (!result.success)
        return false;

    if (objectId)
        *objectId = result.objectId;

    return true;
}

void MirEngineViewportOrbit(void* viewport, double deltaX, double deltaY)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->beginOrbit(0.0, 0.0);
    native->runtime->move(static_cast<mir::Scalar>(deltaX), static_cast<mir::Scalar>(deltaY));
    native->runtime->endInteraction();
}

void MirEngineViewportPan(void* viewport, double deltaX, double deltaY)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->beginPan(0.0, 0.0);
    native->runtime->move(static_cast<mir::Scalar>(deltaX), static_cast<mir::Scalar>(deltaY));
    native->runtime->endInteraction();
}

void MirEngineViewportZoom(void* viewport, double delta)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->zoom(static_cast<mir::Scalar>(delta));
}

} // extern "C"
