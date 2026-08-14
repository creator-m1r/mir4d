#include "MirEngineExports.h"

#include "../../Platform/macOS/OpenGL/MacOpenGLContext.h"
#include "../../Rendering/OpenGL/OpenGLRenderer.h"
#include "../../Rendering/OpenGL/OpenGLContext.h"

#include "../Viewport/ViewportRuntime.hpp"
#include "../Geometry/Scene/Scene.hpp"

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
};

NativeViewport* asViewport(void* handle) noexcept
{
    return static_cast<NativeViewport*>(handle);
}

const char* emptyError()
{
    return "Unknown MirEngine error";
}

}


// ============================================================
// C ABI
// ============================================================

extern "C"
{

// ------------------------------------------------------------
// OpenGL
// ------------------------------------------------------------

void* MirEngineCreateMacOpenGLContext(
    void* view,
    MirEngineSize2D size
)
{
    if (!view)
        return nullptr;

    auto* context = new MacOpenGLContext();

    const Size2D nativeSize{
        std::max(size.width, 1u),
        std::max(size.height, 1u)
    };

    if (!context->initialize(view, nativeSize))
    {
        delete context;
        return nullptr;
    }

    return context;
}


void MirEngineDestroyOpenGLContext(void* context)
{
    if (!context)
        return;

    delete static_cast<OpenGLContext*>(context);
}


// ------------------------------------------------------------
// Renderer
// ------------------------------------------------------------

void* MirEngineCreateOpenGLRenderer(void* context)
{
    if (!context)
        return nullptr;

    return new OpenGLRenderer(
        static_cast<OpenGLContext*>(context)
    );
}


bool MirEngineInitializeRenderer(void* renderer)
{
    if (!renderer)
        return false;

    return static_cast<OpenGLRenderer*>(
        renderer
    )->initialize();
}


void MirEngineDestroyRenderer(void* renderer)
{
    if (!renderer)
        return;

    delete static_cast<OpenGLRenderer*>(renderer);
}


// ------------------------------------------------------------
// Viewport
// ------------------------------------------------------------

void* MirEngineCreateViewport(
    void* renderer,
    uint32_t width,
    uint32_t height
)
{
    if (!renderer)
        return nullptr;

    const uint32_t safeWidth =
        std::max(width, 1u);

    const uint32_t safeHeight =
        std::max(height, 1u);

    auto native =
        std::make_unique<NativeViewport>();

    native->scene =
        std::make_unique<mir::Scene>();

    auto* nativeRenderer =
        static_cast<OpenGLRenderer*>(renderer);

    native->runtime =
        std::make_unique<mir::ViewportRuntime>(
            nativeRenderer
        );

    native->runtime->setScene(
        native->scene.get()
    );

    native->runtime->resize(
        safeWidth,
        safeHeight
    );

    auto& camera =
        native->runtime->state().camera;

    camera.setPerspective(
        mir::Scalar(0.7853981633974483),
        mir::Scalar(safeWidth) /
        mir::Scalar(safeHeight),
        mir::Scalar(0.1),
        mir::Scalar(500.0)
    );

    camera.setTarget({
        0.0,
        0.0,
        0.0
    });

    camera.setOrbit(
        mir::Scalar(0.8),
        mir::Scalar(1.2),
        mir::Scalar(12.0)
    );

    return native.release();
}


void MirEngineDestroyViewport(void* viewport)
{
    delete asViewport(viewport);
}


void MirEngineResize(
    void* viewport,
    uint32_t width,
    uint32_t height
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->resize(
        std::max(width, 1u),
        std::max(height, 1u)
    );
}


void MirEngineRender(void* viewport)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->update(0.0);
    native->runtime->render();
}


// ------------------------------------------------------------
// Camera
// ------------------------------------------------------------

void MirEngineGetCameraOrientation(
    void* viewport,
    float* theta,
    float* phi,
    float* distance
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    const auto& camera =
        native->runtime->state().camera;

    // Если Camera API в текущей версии MirEngine
    // ещё не предоставляет getter — оставляем
    // безопасные значения.
    if (theta)
        *theta = 0.8f;

    if (phi)
        *phi = 1.2f;

    if (distance)
        *distance = 12.0f;
}


void MirEngineSetCameraOrientation(
    void* viewport,
    float theta,
    float phi,
    float distance
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    auto& camera =
        native->runtime->state().camera;

    camera.setOrbit(
        mir::Scalar(theta),
        mir::Scalar(phi),
        mir::Scalar(distance)
    );
}


// ------------------------------------------------------------
// Navigation
// ------------------------------------------------------------

void MirEngineViewportMouseDown(
    void* viewport,
    int button,
    float x,
    float y
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    constexpr int leftButton = 0;
    constexpr int middleButton = 1;
    constexpr int rightButton = 2;

    if (button == leftButton)
    {
        native->runtime->beginOrbit(x, y);
    }
    else if (button == middleButton)
    {
        native->runtime->beginPan(x, y);
    }
    else if (button == rightButton)
    {
        native->runtime->beginPan(x, y);
    }
}


void MirEngineViewportMouseUp(
    void* viewport,
    int button,
    float x,
    float y
)
{
    (void)button;
    (void)x;
    (void)y;

    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->endInteraction();
}


void MirEngineViewportMouseMove(
    void* viewport,
    float x,
    float y
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->move(x, y);
}


void MirEngineViewportScroll(
    void* viewport,
    float delta
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->zoom(delta);
}


void MirEngineViewportClick(
    void* viewport,
    float x,
    float y,
    bool addToSelection
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->selectAt(
        x,
        y,
        addToSelection
    );
}


// ------------------------------------------------------------
// Selection
// ------------------------------------------------------------

uint64_t MirEngineGetSelectedObjectId(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return 0;

    return 0;
}


// ------------------------------------------------------------
// Geometry
// ------------------------------------------------------------

bool MirEngineCreateBox(
    void* viewport,
    double width,
    double depth,
    double height,
    uint64_t* objectId
)
{
    (void)viewport;
    (void)width;
    (void)depth;
    (void)height;

    if (objectId)
        *objectId = 0;

    return false;
}


bool MirEngineImportMesh(
    void* viewport,
    const char* path
)
{
    (void)viewport;
    (void)path;

    return false;
}


bool MirEngineExportStl(
    void* viewport,
    const char* path,
    bool selectionOnly
)
{
    (void)viewport;
    (void)path;
    (void)selectionOnly;

    return false;
}


// ------------------------------------------------------------
// Errors
// ------------------------------------------------------------

const char* MirEngineGetLastError(
    void* viewport
)
{
    (void)viewport;
    return emptyError();
}

}
