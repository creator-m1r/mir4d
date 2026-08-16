#include "MirEngineExports.h"

#include "../../Platform/macOS/OpenGL/MacOpenGLContext.h"
#include "../../Rendering/OpenGL/OpenGLRenderer.h"
#include "../../Rendering/OpenGL/OpenGLContext.h"
#include "../../Rendering/Material/MaterialLibrary.hpp"

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
#include <variant>
#include "../../Sketch/SketchDocument.hpp"
#include "../../Sketch/SketchDocumentSolver.hpp"

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
// Work planes (ТЗ Этап 1)
// ------------------------------------------------------------

void MirEngineSetPlanes(void* renderer,
                        int count,
                        const uint32_t* ids,
                        const float* origins,
                        const float* normals,
                        const float* xAxes,
                        const float* yAxes,
                        const float* colors,
                        const float* sizes,
                        const bool* active,
                        const bool* selected)
{
    if (!renderer)
        return;

    auto* native = static_cast<OpenGLRenderer*>(renderer);
    std::vector<MirEngine::Rendering::PlaneRenderData> data;
    if (count > 0 && ids && origins && normals && xAxes && yAxes && colors && sizes && active && selected)
    {
        data.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i)
        {
            const int o = i * 3;
            MirEngine::Rendering::PlaneRenderData p;
            p.id = ids[i];
            p.origin[0] = origins[o + 0];
            p.origin[1] = origins[o + 1];
            p.origin[2] = origins[o + 2];
            p.normal[0] = normals[o + 0];
            p.normal[1] = normals[o + 1];
            p.normal[2] = normals[o + 2];
            p.xAxis[0] = xAxes[o + 0];
            p.xAxis[1] = xAxes[o + 1];
            p.xAxis[2] = xAxes[o + 2];
            p.yAxis[0] = yAxes[o + 0];
            p.yAxis[1] = yAxes[o + 1];
            p.yAxis[2] = yAxes[o + 2];
            p.color[0] = colors[o + 0];
            p.color[1] = colors[o + 1];
            p.color[2] = colors[o + 2];
            p.size = sizes[i];
            p.active = active[i];
            p.selected = selected[i];
            data.push_back(p);
        }
    }
    native->setPlanes(data);
}


// ------------------------------------------------------------
// Sketch overlay (ТЗ Этап 2)
// ------------------------------------------------------------

void MirEngineSetSketch(void* renderer,
                        int segmentCount,
                        const float* ax,
                        const float* ay,
                        const float* bx,
                        const float* by,
                        const float* colors,
                        const float* origin,
                        const float* xAxis,
                        const float* yAxis)
{
    auto* native = static_cast<OpenGLRenderer*>(renderer);
    if (!native)
        return;

    std::vector<MirEngine::Rendering::SketchRenderData> data;
    if (segmentCount > 0 && ax && ay && bx && by && colors && origin && xAxis && yAxis)
    {
        MirEngine::Rendering::SketchRenderData sk;
        sk.origin[0] = origin[0]; sk.origin[1] = origin[1]; sk.origin[2] = origin[2];
        sk.xAxis[0] = xAxis[0]; sk.xAxis[1] = xAxis[1]; sk.xAxis[2] = xAxis[2];
        sk.yAxis[0] = yAxis[0]; sk.yAxis[1] = yAxis[1]; sk.yAxis[2] = yAxis[2];
        sk.segments.reserve(static_cast<std::size_t>(segmentCount));
        for (int i = 0; i < segmentCount; ++i)
        {
            MirEngine::Rendering::SketchSegment2D seg;
            seg.ax = ax[i]; seg.ay = ay[i];
            seg.bx = bx[i]; seg.by = by[i];
            const int c = i * 3;
            seg.color[0] = colors[c + 0];
            seg.color[1] = colors[c + 1];
            seg.color[2] = colors[c + 2];
            sk.segments.push_back(seg);
        }
        data.push_back(sk);
    }
    native->setSketch(data);
}


// ------------------------------------------------------------
// Work plane store (ТЗ Этап 1)
// ------------------------------------------------------------

#include "../Planes/PlaneStore.hpp"
#include "../Planes/PlaneFactory.hpp"

namespace
{
    void fillPlaneColors(const mir::Plane& plane,
                         float color[3])
    {
        switch (plane.type())
        {
            case mir::PlaneType::BaseXY:
                color[0] = 0.0f; color[1] = 0.9f; color[2] = 1.0f; break;
            case mir::PlaneType::BaseXZ:
                color[0] = 0.2f; color[1] = 1.0f; color[2] = 0.4f; break;
            case mir::PlaneType::BaseYZ:
                color[0] = 1.0f; color[1] = 0.6f; color[2] = 0.1f; break;
            default:
                color[0] = 1.0f; color[1] = 0.85f; color[2] = 0.2f; break;
        }
    }
}

void* MirEngineCreatePlaneStore(void)
{
    return new mir::PlaneStore();
}

void MirEngineDestroyPlaneStore(void* store)
{
    delete static_cast<mir::PlaneStore*>(store);
}

void MirEnginePlaneStoreAddBasePlanes(void* store)
{
    if (store)
        static_cast<mir::PlaneStore*>(store)->ensureBasePlanes();
}

uint32_t MirEnginePlaneStoreCreateOffsetPlane(void* store,
                                              uint32_t basePlane,
                                              double offset,
                                              double angleDeg)
{
    if (!store)
        return 0;
    auto* s = static_cast<mir::PlaneStore*>(store);
    auto base = s->find(basePlane);
    if (!base)
        return 0;
    auto plane = mir::PlaneFactory::createOffset(*base, offset, angleDeg);
    if (!s->add(plane))
        return 0;
    return plane->id();
}

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
                                bool* selected)
{
    if (!store)
        return 0;
    auto* s = static_cast<mir::PlaneStore*>(store);
    const auto planes = s->list();
    const int count = static_cast<int>(planes.size());
    if (maxCount <= 0)
        return count;
    const int n = std::min(count, maxCount);
    for (int i = 0; i < n; ++i)
    {
        const auto& p = planes[static_cast<std::size_t>(i)];
        const int o = i * 3;
        ids[i] = p->id();
        origins[o + 0] = static_cast<float>(p->origin().x);
        origins[o + 1] = static_cast<float>(p->origin().y);
        origins[o + 2] = static_cast<float>(p->origin().z);
        normals[o + 0] = static_cast<float>(p->normal().x);
        normals[o + 1] = static_cast<float>(p->normal().y);
        normals[o + 2] = static_cast<float>(p->normal().z);
        xAxes[o + 0] = static_cast<float>(p->xAxis().x);
        xAxes[o + 1] = static_cast<float>(p->xAxis().y);
        xAxes[o + 2] = static_cast<float>(p->xAxis().z);
        const mir::Vector3 y = p->yAxis();
        yAxes[o + 0] = static_cast<float>(y.x);
        yAxes[o + 1] = static_cast<float>(y.y);
        yAxes[o + 2] = static_cast<float>(y.z);
        float c[3];
        fillPlaneColors(*p, c);
        colors[o + 0] = c[0];
        colors[o + 1] = c[1];
        colors[o + 2] = c[2];
        sizes[i] = 10.0f;
        active[i] = false;
        selected[i] = false;
    }
    return count;
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


void MirEngineSetCameraProjection(
    void* viewport,
    int projection
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->setProjection(
        projection == 0
            ? mir::CameraProjection::Perspective
            : mir::CameraProjection::Orthographic
    );
}


int MirEngineGetCameraProjection(void* viewport)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return 0;

    return native->runtime->state().camera.projection() ==
            mir::CameraProjection::Orthographic
        ? 1
        : 0;
}


void MirEngineSetCameraFov(
    void* viewport,
    float fovYRadians
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->state().camera.setFovY(
        mir::Scalar(fovYRadians)
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

    auto& camera =
        native->runtime->state().camera;

    mir::Scalar distance;
    if (camera.projection() == mir::CameraProjection::Orthographic)
    {
        // In orthographic mode the visible half-height equals the distance,
        // so fit the whole extent with a 35% margin.
        distance = extent * mir::Scalar(0.5) * mir::Scalar(1.35);
    }
    else
    {
        const mir::Scalar fov = camera.fovY();
        distance = (extent * mir::Scalar(0.5)) /
                   std::tan(fov * mir::Scalar(0.5)) *
                   mir::Scalar(1.35);
    }

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

    native->runtime->handleMouseDown(button, x, y);
}


void MirEngineViewportMouseUp(
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

    native->runtime->handleMouseUp(button, x, y);
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

    native->runtime->handleMouseMove(x, y);
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


void MirEngineViewportZoomAt(
    void* viewport,
    float delta,
    float x,
    float y
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->zoomAt(delta, x, y);
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


void MirEngineViewportOrbit(
    void* viewport,
    float dx,
    float dy
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->orbitBy(dx, dy);
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


void MirEngineViewportHover(
    void* viewport,
    float x,
    float y
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->updateHover(x, y);
}


void MirEngineViewportHoverClear(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->clearHover();
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


// Deletes the primary selection through the canonical Scene API.
// The renderer observes the scene change; nothing is removed from the
// renderer directly.
bool MirEngineDeleteSelectedObject(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return false;

    return native->runtime->deleteSelectedObject();
}


// Clears the selection set without modifying the scene.
void MirEngineClearSelection(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->clearSelection();
}


// Aborts an active drag and restores the drag-start transform (Esc).
void MirEngineViewportDragCancel(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->cancelDrag();
}


bool MirEngineUndo(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return false;

    return native->runtime->undo();
}


bool MirEngineRedo(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return false;

    return native->runtime->redo();
}


bool MirEngineCanUndo(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return false;

    return native->runtime->canUndo();
}


bool MirEngineCanRedo(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return false;

    return native->runtime->canRedo();
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
// Materials (procedural MaterialLibrary, no textures)
// ------------------------------------------------------------

int32_t MirEngineMaterialCount(void)
{
    return MirEngine::Rendering::MaterialLibrary::count();
}


bool MirEngineMaterialName(
    int32_t materialId,
    char* buffer,
    size_t bufferSize
)
{
    if (!buffer || bufferSize == 0)
        return false;

    const auto id =
        static_cast<MirEngine::Rendering::MaterialId>(materialId);
    const char* name =
        MirEngine::Rendering::MaterialLibrary::name(id);

    size_t index = 0;
    while (name[index] != '\0' && index + 1 < bufferSize)
    {
        buffer[index] = name[index];
        ++index;
    }
    buffer[index] = '\0';
    return true;
}


bool MirEngineSetObjectMaterial(
    void* viewport,
    uint64_t objectId,
    int32_t materialId
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
    {
        setLastError(native, "MIR4D material: viewport is not ready");
        return false;
    }

    const auto id =
        static_cast<MirEngine::Rendering::MaterialId>(materialId);
    native->runtime->setObjectMaterial(
        mir4d::ObjectId{objectId},
        id
    );

    setLastError(native, nullptr);
    return true;
}


bool MirEngineGetOpenGLDiagnostics(
    void* renderer,
    char* buffer,
    size_t bufferSize
)
{
    if (!renderer || !buffer || bufferSize == 0)
        return false;

    auto* native = static_cast<OpenGLRenderer*>(renderer);
    const std::string report = native->diagnosticsReport();

    size_t index = 0;
    while (index + 1 < bufferSize && index < report.size())
    {
        buffer[index] = report[index];
        ++index;
    }
    buffer[index] = '\0';
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

// ------------------------------------------------------------
// Sketch solver (universal constraint solver)
// ------------------------------------------------------------

void* MirEngineSketchCreateDocument(void)
{
    return reinterpret_cast<void*>(new mir::SketchDocument());
}

void MirEngineSketchDestroyDocument(void* doc)
{
    delete reinterpret_cast<mir::SketchDocument*>(doc);
}

uint32_t MirEngineSketchAddLine(void* doc, float x1, float y1, float x2, float y2)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return 0;
    return d->geometry().addLine({x1, y1}, {x2, y2});
}

uint32_t MirEngineSketchAddCircle(void* doc, float cx, float cy, float r)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return 0;
    return d->geometry().addCircle({cx, cy}, r);
}

uint32_t MirEngineSketchAddConstraint(void* doc, int type, uint32_t g1, uint32_t g2, double value)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return 0;
    const auto ct = static_cast<mir::SketchConstraintType>(type);
    return d->constraints().add(ct, g1, g2, value);
}

bool MirEngineSketchSolve(void* doc)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return false;
    const auto result = mir::SketchDocumentSolver::solve(*d);
    return result.converged;
}

bool MirEngineSketchGetLine(void* doc, uint32_t id, float* x1, float* y1, float* x2, float* y2)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return false;
    const auto* g = d->geometry().find(id);
    if (!g) return false;
    if (const auto* l = std::get_if<mir::SketchLine2D>(g))
    {
        if (x1) *x1 = static_cast<float>(l->start.x);
        if (y1) *y1 = static_cast<float>(l->start.y);
        if (x2) *x2 = static_cast<float>(l->end.x);
        if (y2) *y2 = static_cast<float>(l->end.y);
        return true;
    }
    return false;
}

bool MirEngineSketchGetCircle(void* doc, uint32_t id, float* cx, float* cy, float* r)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return false;
    const auto* g = d->geometry().find(id);
    if (!g) return false;
    if (const auto* cc = std::get_if<mir::SketchCircle2D>(g))
    {
        if (cx) *cx = static_cast<float>(cc->center.x);
        if (cy) *cy = static_cast<float>(cc->center.y);
        if (r) *r = static_cast<float>(cc->radius);
        return true;
    }
    return false;
}

}
