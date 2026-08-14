#include "MirEngineExports.h"

#include "../../Platform/macOS/OpenGL/MacOpenGLContext.h"
#include "../../Rendering/OpenGL/OpenGLRenderer.h"
#include "../../Rendering/OpenGL/OpenGLContext.h"

#include "../Viewport/ViewportRuntime.hpp"
#include "../Geometry/Scene/Scene.hpp"
#include "../Geometry/Model/Model.hpp"
#include "../Geometry/Tessellation/TriangleMesh.hpp"
#include "../Math/Transform.hpp"
#include "../BRep/Commands/BRepSceneBridge.hpp"
#include "../Document/Document.hpp"
#include "../IO/Mesh/StlImporter.hpp"
#include "../IO/Mesh/StlExporter.hpp"
#include "../IO/ExportOptions.hpp"
#include "../IO/ImportOptions.hpp"
#include "../IO/ImportService.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

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
    std::string lastError;
};

NativeViewport* asViewport(void* handle) noexcept
{
    return static_cast<NativeViewport*>(handle);
}

void setLastError(NativeViewport* native, const char* message) noexcept
{
    if (!native)
        return;

    if (message != nullptr && *message != '\0')
        native->lastError = message;
    else
        native->lastError.clear();
}

const char* errorMessage(NativeViewport* native) noexcept
{
    if (native && !native->lastError.empty())
        return native->lastError.c_str();
    return nullptr;
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

    if (theta)
        *theta = static_cast<float>(camera.theta());

    if (phi)
        *phi = static_cast<float>(camera.phi());

    if (distance)
        *distance = static_cast<float>(camera.distance());
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
// Camera presets (navigation sphere)
// ------------------------------------------------------------

namespace
{
constexpr double kPresetPhi = 1.1;
constexpr double kPresetDistance = 12.0;
} // namespace

void MirEngineSetActiveCameraPreset(
    void* viewport,
    int preset
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    auto& camera =
        native->runtime->state().camera;

    constexpr double kThetaFront = 0.0;
    constexpr double kThetaBack = 3.14159265358979323846;
    constexpr double kThetaLeft = -3.14159265358979323846 * 0.5;
    constexpr double kThetaRight = 3.14159265358979323846 * 0.5;
    constexpr double kThetaTop = 0.0;
    constexpr double kThetaBottom = 3.14159265358979323846 * 0.0;
    constexpr double kThetaIsometric = 0.78539816339744830962;

    double theta = 0.0;
    double phi = kPresetPhi;
    double distance = kPresetDistance;

    switch (preset)
    {
    case 0: theta = kThetaFront;      phi = kPresetPhi; break;
    case 1: theta = kThetaBack;       phi = kPresetPhi; break;
    case 2: theta = kThetaLeft;       phi = kPresetPhi; break;
    case 3: theta = kThetaRight;      phi = kPresetPhi; break;
    case 4: theta = kThetaTop;        phi = 1e-4; break;
    case 5: theta = kThetaBottom;     phi = 3.14159265358979323846 - 1e-4; break;
    case 6: theta = kThetaIsometric;  phi = 0.61547970867038739117; break;
    default: return;
    }

    camera.setOrbit(
        mir::Scalar(theta),
        mir::Scalar(phi),
        mir::Scalar(distance)
    );
}


// ------------------------------------------------------------
// Fit All
// ------------------------------------------------------------

void MirEngineFitViewport(void* viewport)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime || !native->scene)
        return;

    bool hasBounds = false;
    mir::Point3 minPoint{
        std::numeric_limits<mir::Scalar>::max(),
        std::numeric_limits<mir::Scalar>::max(),
        std::numeric_limits<mir::Scalar>::max()};
    mir::Point3 maxPoint{
        std::numeric_limits<mir::Scalar>::lowest(),
        std::numeric_limits<mir::Scalar>::lowest(),
        std::numeric_limits<mir::Scalar>::lowest()};

    for (const auto& node : native->scene->nodes())
    {
        if (!node || !node->model())
            continue;

        const mir::TriangleMesh3& mesh =
            node->model()->mesh();

        if (!mesh.isValid())
            continue;

        const mir::Transform& transform =
            node->transform();

        for (const auto& vertex : mesh.vertices)
        {
            const mir::Point3 world =
                transform.transformPoint(vertex);

            minPoint.x = std::min(minPoint.x, world.x);
            minPoint.y = std::min(minPoint.y, world.y);
            minPoint.z = std::min(minPoint.z, world.z);

            maxPoint.x = std::max(maxPoint.x, world.x);
            maxPoint.y = std::max(maxPoint.y, world.y);
            maxPoint.z = std::max(maxPoint.z, world.z);
        }

        hasBounds = true;
    }

    if (!hasBounds)
        return;

    const mir::Point3 center{
        (minPoint.x + maxPoint.x) * mir::Scalar(0.5),
        (minPoint.y + maxPoint.y) * mir::Scalar(0.5),
        (minPoint.z + maxPoint.z) * mir::Scalar(0.5)};

    const mir::Scalar extent = std::max({
        maxPoint.x - minPoint.x,
        maxPoint.y - minPoint.y,
        maxPoint.z - minPoint.z,
        mir::Scalar(1e-6)
    });

    constexpr mir::Scalar kFovY =
        mir::Scalar(0.7853981633974483);

    const mir::Scalar distance =
        (extent * mir::Scalar(0.5)) /
        std::tan(kFovY * mir::Scalar(0.5)) *
        mir::Scalar(1.35);

    auto& camera =
        native->runtime->state().camera;

    camera.setTarget(center);
    camera.setOrbit(
        camera.theta(),
        camera.phi(),
        std::max(distance, mir::Scalar(0.1))
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


void MirEngineViewportPan(
    void* viewport,
    float dx,
    float dy
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->panBy(dx, dy);
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

    return static_cast<uint64_t>(
        native->runtime->state().selection.primary()
    );
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
    auto* native =
        asViewport(viewport);

    if (objectId)
        *objectId = 0;

    if (!native || !native->runtime || !native->scene)
    {
        setLastError(native, "MIR4D create box: viewport is not ready");
        return false;
    }

    if (!(width > 0.0) || !(depth > 0.0) || !(height > 0.0))
    {
        setLastError(native, "MIR4D create box: dimensions must be positive");
        return false;
    }

    mir::BRepModel brep;
    const mir4d::BRepSceneInsertResult inserted =
        mir4d::BRepSceneBridge::createBox(
            *native->scene,
            brep,
            mir::Scalar(width),
            mir::Scalar(depth),
            mir::Scalar(height)
        );

    if (!inserted.success)
    {
        setLastError(native, "MIR4D create box: B-Rep pipeline failed");
        return false;
    }

    if (objectId)
        *objectId = static_cast<uint64_t>(inserted.objectId);

    setLastError(native, nullptr);
    return true;
}


bool MirEngineImportMesh(
    void* viewport,
    const char* path
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->scene)
    {
        setLastError(native, "MIR4D import: viewport is not ready");
        return false;
    }

    if (path == nullptr || *path == '\0')
    {
        setLastError(native, "MIR4D import: path is empty");
        return false;
    }

    const mir::io::ImportResult imported =
        mir::io::ImportService{}.importFile(path);

    if (!imported.ok())
    {
        setLastError(
            native,
            imported.error.empty()
                ? "MIR4D import: parsing failed"
                : imported.error.c_str());
        return false;
    }

    auto model = std::make_shared<mir::Model>();
    model->setMesh(*imported.mesh);

    const auto node = native->scene->createNode(std::move(model));
    if (!node)
    {
        setLastError(native, "MIR4D import: failed to add mesh to scene");
        return false;
    }

    setLastError(native, nullptr);
    return true;
}


bool MirEngineExportStl(
    void* viewport,
    const char* path,
    bool selectionOnly
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->scene)
    {
        setLastError(native, "MIR4D export: viewport is not ready");
        return false;
    }

    if (path == nullptr || *path == '\0')
    {
        setLastError(native, "MIR4D export: path is empty");
        return false;
    }

    // The canonical STL writer consumes a mir4d::Document. Reuse it by
    // staging the viewport meshes into a temporary export document instead
    // of duplicating the STL serialization in this ABI layer.
    mir4d::Document document{"MIR4D Viewport Export"};
    mir::Scene& targetScene = document.scene();

    mir::io::ExportOptions options;
    options.selectionOnly = selectionOnly;
    if (selectionOnly)
        options.selection =
            native->runtime->state().selection.ids();

    for (const auto& node : native->scene->nodes())
    {
        if (!node || !node->model())
            continue;

        if (selectionOnly)
        {
            bool included = false;
            for (const mir4d::ObjectId selected : options.selection)
            {
                if (selected == node->id())
                {
                    included = true;
                    break;
                }
            }
            if (!included)
                continue;
        }

        auto copy =
            std::make_shared<mir::ModelNode>(node->model());

        copy->setTransform(node->transform());
        const auto added = targetScene.add(copy);

        if (!added)
        {
            setLastError(
                native,
                "MIR4D export: failed to stage scene object");
            return false;
        }
    }

    const mir::io::ExportResult result =
        mir::io::StlExporter().exportTo(
            path,
            document,
            options
        );

    if (!result.error.empty())
    {
        setLastError(native, result.error.c_str());
        return false;
    }

    setLastError(native, nullptr);
    return true;
}


// ------------------------------------------------------------
// Errors
// ------------------------------------------------------------

const char* MirEngineGetLastError(
    void* viewport
)
{
    return errorMessage(asViewport(viewport));
}

}
