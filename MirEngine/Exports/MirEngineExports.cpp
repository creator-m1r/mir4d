#include "MirEngineExports.h"

#include "../../Platform/macOS/OpenGL/MacOpenGLContext.h"
#include "../../Rendering/OpenGL/OpenGLRenderer.h"
#include "../../Rendering/OpenGL/OpenGLContext.h"
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
    // These three objects form the complete native OpenGL viewport pipeline:
    // NSView-owned context -> MirEngine renderer -> MirEngine Scene.
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
        mir::Scalar(safeWidth) / mir::Scalar(safeHeight),
        mir::Scalar(0.1),
        mir::Scalar(500.0)
    );
    camera.setTarget({0.0, 0.0, 0.0});
    camera.setOrbit(mir::Scalar(0.8), mir::Scalar(1.2), mir::Scalar(12.0));

    return native.release();
}

void MirEngineDestroyViewport(void* viewport)
{
    delete asViewport(viewport);
}

void MirEngineRender(void* viewport)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->update(0.0);
    native->runtime->render();
}

void MirEngineResize(void* viewport, uint32_t width, uint32_t height)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->resize(std::max(width, 1u), std::max(height, 1u));
}

bool MirEngineCreateBox(
    void* viewport,
    double width,
    double depth,
    double height,
    uint64_t* objectId)
{
    if (objectId)
        *objectId = 0;

    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene || !native->brep)
        return false;

    if (!(width > 0.0) || !(depth > 0.0) || !(height > 0.0))
        return false;

    const auto result = mir4d::BRepSceneBridge::createBox(
        *native->scene,
        *native->brep,
        static_cast<mir::Scalar>(width),
        static_cast<mir::Scalar>(depth),
        static_cast<mir::Scalar>(height)
    );

    if (!result.success)
        return false;

    if (objectId)
        *objectId = static_cast<uint64_t>(result.objectId);

    // The viewport renderer references native->scene directly, so no copy or
    // Swift-side mesh is required. The next OpenGL frame renders the newly
    // inserted ModelNode through MirEngine::ViewportRuntime.
    return true;
}

void MirEngineViewportMouseDown(void* viewport, int button, float x, float y)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    constexpr int kLeftMouseButton = 0;
    constexpr int kMiddleMouseButton = 1;
    constexpr int kRightMouseButton = 2;

    if (button == kLeftMouseButton)
        native->runtime->beginOrbit(x, y);
    else if (button == kMiddleMouseButton || button == kRightMouseButton)
        native->runtime->beginPan(x, y);
}

void MirEngineViewportMouseUp(void* viewport, int button, float x, float y)
{
    (void)button;
    (void)x;
    (void)y;

    auto* native = asViewport(viewport);
    if (native && native->runtime)
        native->runtime->endInteraction();
}

void MirEngineViewportMouseMove(void* viewport, float x, float y)
{
    auto* native = asViewport(viewport);
    if (native && native->runtime)
        native->runtime->move(x, y);
}

void MirEngineViewportScroll(void* viewport, float delta)
{
    auto* native = asViewport(viewport);
    if (native && native->runtime)
        native->runtime->zoom(delta);
}

void MirEngineViewportClick(void* viewport, float x, float y, bool addToSelection)
{
    auto* native = asViewport(viewport);
    if (native && native->runtime)
        native->runtime->selectAt(x, y, addToSelection);
}

} // extern "C"
