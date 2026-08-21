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
#include "../Interaction/RayPicker.hpp"
#include "../IO/Mesh/StlImporter.hpp"
#include "../IO/Mesh/StlExporter.hpp"
#include "../IO/Step/StepImporter.hpp"
#include "../IO/Step/StepExporter.hpp"
#include "../IO/Step/BRepStepBridge.hpp"
#include "../BRep/Converters/BRepToModel.hpp"
#include "../BRep/Converters/BRepMerge.hpp"
#include "../IO/ExportOptions.hpp"
#include "../IO/ImportOptions.hpp"
#include "../IO/ImportService.hpp"
#include <variant>
#include <chrono>
#include <cstdio>
#include <iostream>
#include "../../Sketch/SketchDocument.hpp"
#include "../../Sketch/SketchDocumentSolver.hpp"
#include "../../VFX/EffectSystem.hpp"
#include "../../VFX/Effect.hpp"

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

// Transform (C ABI ↔ mir4d::Transform) helpers. Defined as file-static C++
// functions so they never inherit the C linkage of the export stubs below.
static MirTransform toMirTransform(const mir4d::Transform& t) noexcept
{
    MirTransform m{};
    m.px = static_cast<double>(t.position.x);
    m.py = static_cast<double>(t.position.y);
    m.pz = static_cast<double>(t.position.z);
    m.qx = static_cast<double>(t.rotation.x);
    m.qy = static_cast<double>(t.rotation.y);
    m.qz = static_cast<double>(t.rotation.z);
    m.qw = static_cast<double>(t.rotation.w);
    m.sx = static_cast<double>(t.scale.x);
    m.sy = static_cast<double>(t.scale.y);
    m.sz = static_cast<double>(t.scale.z);
    return m;
}

static mir4d::Transform fromMirTransform(const MirTransform& m) noexcept
{
    mir4d::Transform t{};
    t.position = {m.px, m.py, m.pz};
    t.rotation = mir::Quaternion(m.qx, m.qy, m.qz, m.qw);
    t.scale = {m.sx, m.sy, m.sz};
    return t;
}

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


void MirEngineSetOpenGLContextView(void* context, void* view)
{
    if (!context)
        return;

    static_cast<OpenGLContext*>(context)->setView(view);
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

void MirEngineSetCursor(void* renderer, float ndcX, float ndcY, bool active)
{
    if (!renderer)
        return;
    auto* native = static_cast<OpenGLRenderer*>(renderer);
    native->setCursor(ndcX, ndcY, active);
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
        // Промышленная палитра: плоскость окрашена по оси своей нормали
        // (X=красный, Y=зелёный, Z=синий), приглушённые тона.
        switch (plane.type())
        {
            case mir::PlaneType::BaseXY:
                color[0] = 0.18f; color[1] = 0.40f; color[2] = 0.80f; break; // нормаль Z — синий
            case mir::PlaneType::BaseXZ:
                color[0] = 0.22f; color[1] = 0.62f; color[2] = 0.38f; break; // нормаль Y — зелёный
            case mir::PlaneType::BaseYZ:
                color[0] = 0.78f; color[1] = 0.30f; color[2] = 0.26f; break; // нормаль X — красный
            default:
                color[0] = 0.55f; color[1] = 0.58f; color[2] = 0.66f; break; // пользовательские — серый
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
        active[i] = (p->id() == mir::kBasePlaneXY);
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

    std::fprintf(stderr, "[MIR4D-DBG %lldms] %s\n", (long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(), "CreateViewport: scene");
    native->scene =
        std::make_unique<mir::Scene>();

    auto* nativeRenderer =
        static_cast<OpenGLRenderer*>(renderer);

    std::fprintf(stderr, "[MIR4D-DBG %lldms] %s\n", (long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(), "CreateViewport: runtime");
    native->runtime =
        std::make_unique<mir::ViewportRuntime>(
            nativeRenderer
        );

    native->runtime->setScene(
        native->scene.get()
    );

    std::fprintf(stderr, "[MIR4D-DBG %lldms] %s\n", (long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(), "CreateViewport: resize");
    native->runtime->resize(
        safeWidth,
        safeHeight
    );
    std::fprintf(stderr, "[MIR4D-DBG %lldms] %s\n", (long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(), "CreateViewport: resize done");

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
    std::fprintf(stderr, "[MIR4D-DBG %lldms] %s\n", (long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(), "CreateViewport: DONE");

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

// Resolves a camera preset into an orbit (theta, phi, distance).
// Single source of truth for presets: used both by
// MirEngineSetActiveCameraPreset and MirEngineGetCameraPresetOrientation
// (the latter feeds the animated transitions of the navigation sphere).
bool resolveCameraPreset(
    int preset,
    double& theta,
    double& phi,
    double& distance
)
{
    constexpr double kPi = 3.14159265358979323846;
    constexpr double kThetaFront = 0.0;
    constexpr double kThetaBack = kPi;
    constexpr double kThetaLeft = -kPi * 0.5;
    constexpr double kThetaRight = kPi * 0.5;
    constexpr double kThetaIsometric = kPi * 0.25;

    // Diagonal directions of the navigation cube (edges and corners).
    // Direction (x, y, z): x right, y up, z forward.
    // theta = atan2(x, z); phi = acos(y / |v|).
    constexpr double kPhiCornerUp = 0.95531661812450927816;    // acos(1/sqrt(3))
    constexpr double kPhiCornerDown = 2.18627603546528397433;  // pi - acos(1/sqrt(3))
    constexpr double kPhiEdgeUp = kPi * 0.25;                  // acos(1/sqrt(2))
    constexpr double kPhiEdgeDown = kPi * 0.75;
    constexpr double kTheta45 = kPi * 0.25;
    constexpr double kTheta135 = kPi * 0.75;
    constexpr double kThetaM45 = -kPi * 0.25;
    constexpr double kThetaM135 = -kPi * 0.75;

    theta = 0.0;
    phi = kPresetPhi;
    distance = kPresetDistance;

    switch (preset)
    {
    // Faces (standard views)
    case 0: theta = kThetaFront;      phi = kPresetPhi; break;
    case 1: theta = kThetaBack;       phi = kPresetPhi; break;
    case 2: theta = kThetaLeft;       phi = kPresetPhi; break;
    case 3: theta = kThetaRight;      phi = kPresetPhi; break;
    case 4: theta = kThetaFront;      phi = 1e-4; break;
    case 5: theta = kThetaFront;      phi = kPi - 1e-4; break;
    case 6: theta = kThetaIsometric;  phi = 0.61547970867038739117; break;

    // Corners
    case 7:  theta = kThetaM45; phi = kPhiCornerUp;   break; // top-front-left
    case 8:  theta = kTheta45;  phi = kPhiCornerUp;   break; // top-front-right
    case 9:  theta = kThetaM135; phi = kPhiCornerUp;  break; // top-back-left
    case 10: theta = kTheta135; phi = kPhiCornerUp;   break; // top-back-right
    case 11: theta = kThetaM45; phi = kPhiCornerDown; break; // bottom-front-left
    case 12: theta = kTheta45;  phi = kPhiCornerDown; break; // bottom-front-right
    case 13: theta = kThetaM135; phi = kPhiCornerDown; break; // bottom-back-left
    case 14: theta = kTheta135; phi = kPhiCornerDown; break; // bottom-back-right

    // Horizontal edges (diagonal side views)
    case 15: theta = kThetaM45; phi = kPresetPhi; break; // front-left
    case 16: theta = kTheta45;  phi = kPresetPhi; break; // front-right
    case 17: theta = kThetaM135; phi = kPresetPhi; break; // back-left
    case 18: theta = kTheta135; phi = kPresetPhi; break; // back-right

    // Vertical edges
    case 19: theta = kThetaFront;  phi = kPhiEdgeUp;   break; // top-front
    case 20: theta = kThetaBack;   phi = kPhiEdgeUp;   break; // top-back
    case 21: theta = kThetaLeft;   phi = kPhiEdgeUp;   break; // top-left
    case 22: theta = kThetaRight;  phi = kPhiEdgeUp;   break; // top-right
    case 23: theta = kThetaFront;  phi = kPhiEdgeDown; break; // bottom-front
    case 24: theta = kThetaBack;   phi = kPhiEdgeDown; break; // bottom-back
    case 25: theta = kThetaLeft;   phi = kPhiEdgeDown; break; // bottom-left
    case 26: theta = kThetaRight;  phi = kPhiEdgeDown; break; // bottom-right

    default: return false;
    }

    return true;
}
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

    double theta = 0.0;
    double phi = kPresetPhi;
    double distance = kPresetDistance;

    if (!resolveCameraPreset(preset, theta, phi, distance))
        return;

    camera.setOrbit(
        mir::Scalar(theta),
        mir::Scalar(phi),
        mir::Scalar(distance)
    );
}

// Returns the orbit angles of a preset without touching the viewport.
// Used by the navigation sphere to animate camera transitions.
void MirEngineGetCameraPresetOrientation(
    int preset,
    float* theta,
    float* phi,
    float* distance
)
{
    double thetaOut = 0.0;
    double phiOut = kPresetPhi;
    double distanceOut = kPresetDistance;

    if (!resolveCameraPreset(preset, thetaOut, phiOut, distanceOut))
        return;

    if (theta)
        *theta = static_cast<float>(thetaOut);

    if (phi)
        *phi = static_cast<float>(phiOut);

    if (distance)
        *distance = static_cast<float>(distanceOut);
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

void MirEngineSetSelectionFilter(
    void* viewport,
    int mode
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->setPickFilter(mir::makePickFilter(mode));
}

int MirEngineGetSelectionKind(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return 0;

    return static_cast<int>(
        native->runtime->state().selection.kind()
    );
}

uint64_t MirEngineGetSelectionElementId(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return 0;

    return native->runtime->state().selection.elementId();
}

void MirEngineViewportBoxSelect(
    void* viewport,
    float x0,
    float y0,
    float x1,
    float y1,
    bool additive
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->selectInRect(
        x0,
        y0,
        x1,
        y1,
        additive
    );
}

int MirEngineGetSelectionCount(
    void* viewport
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return 0;

    return static_cast<int>(native->runtime->state().multiSelectionCount());
}

uint64_t MirEngineGetSelectionItem(
    void* viewport,
    int index
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return 0;

    return static_cast<uint64_t>(
        native->runtime->state().multiSelectionAt(
            static_cast<std::size_t>(index < 0 ? 0 : index)
        )
    );
}

int MirEngineGetSelectionItemKind(
    void* viewport,
    int index
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return 0;
    return static_cast<int>(
        native->runtime->state().multiSelectionKindAt(
            static_cast<std::size_t>(index < 0 ? 0 : index)
        )
    );
}

uint64_t MirEngineGetSelectionItemElementId(
    void* viewport,
    int index
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return 0;
    return native->runtime->state().multiSelectionElementIdAt(
        static_cast<std::size_t>(index < 0 ? 0 : index)
    );
}

/// Forward declaration: exact B-Rep edge length helper defined below.
static double exactEdgeLength(const mir::Scene::NodePtr& node, std::uint64_t elementId);

/// Returns the geometric metric of an arbitrary element (face area / edge
/// length) addressed by object id + kind + element id. Used by the
/// multi-selection inspector to show per-item metrics.
bool MirEngineGetElementMetric(
    void* viewport,
    uint64_t objectId,
    int kind,
    uint64_t elementId,
    double* outArea,
    double* outLength
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene)
        return false;

    const auto node = native->scene->find(objectId);
    if (!node || !node->model() || node->model()->mesh().empty())
        return false;

    const auto& mesh = node->model()->mesh();
    const mir::Transform transform = node->transform();
    const mir::PickKind pk = static_cast<mir::PickKind>(kind);

    if (pk == mir::PickKind::Face)
    {
        double area = 0.0;
        bool found = false;
        for (const auto& tri : mesh.triangles)
        {
            if (tri.sourceFaceId != elementId)
                continue;
            found = true;
            const mir::Point3 a = transform.transformPoint(mesh.vertices[tri.a]);
            const mir::Point3 b = transform.transformPoint(mesh.vertices[tri.b]);
            const mir::Point3 c = transform.transformPoint(mesh.vertices[tri.c]);
            area += 0.5 * mir::Vector3::cross(b - a, c - a).length();
        }
        if (outArea) *outArea = found ? area : 0.0;
        if (outLength) *outLength = 0.0;
        return found;
    }

    if (pk == mir::PickKind::Edge)
    {
        const double exact = exactEdgeLength(node, elementId);
        if (exact > 0.0)
        {
            if (outLength) *outLength = exact;
            if (outArea) *outArea = 0.0;
            return true;
        }
        const std::size_t ti = static_cast<std::size_t>(elementId / 3);
        const int e = static_cast<int>(elementId % 3);
        if (ti >= mesh.triangles.size())
        {
            if (outArea) *outArea = 0.0;
            if (outLength) *outLength = 0.0;
            return false;
        }
        const auto& tri = mesh.triangles[ti];
        const std::size_t ends[2] = {
            (e == 0) ? tri.a : (e == 1) ? tri.b : tri.c,
            (e == 0) ? tri.b : (e == 1) ? tri.c : tri.a};
        const mir::Point3 a = transform.transformPoint(mesh.vertices[ends[0]]);
        const mir::Point3 b = transform.transformPoint(mesh.vertices[ends[1]]);
        if (outLength) *outLength = (b - a).length();
        if (outArea) *outArea = 0.0;
        return true;
    }

    if (outArea) *outArea = 0.0;
    if (outLength) *outLength = 0.0;
    return false;
}


bool MirEngineDeformSelected(
    void* viewport,
    double x, double y, double z,
    double radius,
    double strength,
    int mode)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene)
        return false;

    const mir4d::ObjectId selected = native->runtime->state().selection.primary();
    if (!mir4d::isValidObjectId(selected))
        return false;

    auto node = native->scene->find(selected);
    if (!node || !node->model() || node->model()->mesh().empty())
        return false;

    mir::TriangleMesh3& mesh = node->model()->mesh();

    const mir::Point3 brush(x, y, z);
    const mir::Point3 local = node->transform().inverse().transformPoint(brush);
    const double r = std::max(radius, 1e-4);

    const bool smooth = (mode == 2);
    int sign = 1;
    if (mode == 1 || mode == 5 || mode == 6)
        sign = -1; // pull, pinch, cut

    if (smooth)
    {
        // One Laplacian relaxation pass over vertices inside the brush.
        std::vector<mir::Vector3> offsets(mesh.vertices.size(), mir::Vector3(0, 0, 0));
        std::vector<int> counts(mesh.vertices.size(), 0);
        for (const auto& t : mesh.triangles)
        {
            const std::size_t ids[3] = { t.a, t.b, t.c };
            for (int e = 0; e < 3; ++e)
            {
                const std::size_t i = ids[e];
                const std::size_t j = ids[(e + 1) % 3];
                const std::size_t k = ids[(e + 2) % 3];
                const double dist = (mesh.vertices[i] - local).length();
                if (dist > r)
                    continue;
                const double tt = 1.0 - dist / r;
                const double falloff = tt * tt * (3.0 - 2.0 * tt);
                const mir::Vector3 vj(mesh.vertices[j].x, mesh.vertices[j].y, mesh.vertices[j].z);
                const mir::Vector3 vk(mesh.vertices[k].x, mesh.vertices[k].y, mesh.vertices[k].z);
                const mir::Vector3 vi(mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z);
                const mir::Vector3 neighbour = (vj + vk) * 0.5;
                offsets[i] += (neighbour - vi) * (sign * strength * falloff);
                counts[i] += 1;
            }
        }
        for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            if (counts[i] > 0)
                mesh.vertices[i] = mesh.vertices[i] + offsets[i] / static_cast<double>(counts[i]);
        }
    }
    else
    {
        for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            const double dist = (mesh.vertices[i] - local).length();
            if (dist > r)
                continue;
            const double tt = 1.0 - dist / r;
            const double falloff = tt * tt * (3.0 - 2.0 * tt);
            mir::Vector3 nrm = mesh.normals.empty() ? mir::Vector3(0, 0, 1) : mesh.normals[i];
            const double nl = nrm.length();
            if (nl < 1e-9)
                nrm = mir::Vector3(0, 0, 1);
            else
                nrm = nrm.normalized();
            mesh.vertices[i] = mesh.vertices[i] + nrm * (sign * strength * falloff);
        }
    }

    // Recompute smooth vertex normals from the deformed triangles.
    if (mesh.normals.size() != mesh.vertices.size())
        mesh.normals.resize(mesh.vertices.size());
    std::vector<mir::Vector3> acc(mesh.vertices.size(), mir::Vector3(0, 0, 0));
    for (const auto& t : mesh.triangles)
    {
        const mir::Vector3 fn = mir::Vector3::cross(
            mesh.vertices[t.b] - mesh.vertices[t.a],
            mesh.vertices[t.c] - mesh.vertices[t.a]);
        acc[t.a] += fn;
        acc[t.b] += fn;
        acc[t.c] += fn;
    }
    for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
    {
        const double l = acc[i].length();
        mesh.normals[i] = l > 1e-9 ? acc[i].normalized() : mir::Vector3(0, 0, 1);
    }

    mesh.markGeometryChanged();
    node->touch();
    return true;
}

/// Real CAD geometry bridge: extracts bounding box, volume, surface area and
/// Computes a compact JSON geometry brief (bounding box, volume, surface area,
/// mesh counts) for an already-resolved scene node.
static bool writeObjectMetrics(const mir::Scene::NodePtr& node,
                               char* outJson,
                               size_t outCapacity)
{
    if (!node || !node->model() || node->model()->mesh().empty())
    {
        std::snprintf(outJson, outCapacity, "{\"hasGeometry\":false}");
        return true;
    }

    const mir::TriangleMesh3& mesh = node->model()->mesh();
    const mir::Point3 bmin = mesh.boundsMin();
    const mir::Point3 bmax = mesh.boundsMax();

    double volume = 0.0;
    double surfaceArea = 0.0;
    for (const auto& tri : mesh.triangles)
    {
        const mir::Point3 v0 = mesh.vertices[tri.a];
        const mir::Point3 v1 = mesh.vertices[tri.b];
        const mir::Point3 v2 = mesh.vertices[tri.c];
        const mir::Vector3 e1 = v1 - v0;
        const mir::Vector3 e2 = v2 - v0;
        const mir::Vector3 cross = mir::Vector3::cross(e1, e2);
        volume += double(v0.x) * cross.x + double(v0.y) * cross.y + double(v0.z) * cross.z;
        surfaceArea += 0.5 * cross.length();
    }
    volume = std::fabs(volume) / 6.0;

    std::snprintf(outJson, outCapacity,
        "{\"hasGeometry\":true,\"objectId\":%llu,"
        "\"sizeX\":%.6g,\"sizeY\":%.6g,\"sizeZ\":%.6g,"
        "\"volume\":%.6g,\"surfaceArea\":%.6g,"
        "\"vertexCount\":%zu,\"faceCount\":%zu,"
        "\"boundsMin\":{\"x\":%.6g,\"y\":%.6g,\"z\":%.6g},"
        "\"boundsMax\":{\"x\":%.6g,\"y\":%.6g,\"z\":%.6g}}",
        static_cast<unsigned long long>(node->id()),
        double(bmax.x - bmin.x), double(bmax.y - bmin.y), double(bmax.z - bmin.z),
        volume, surfaceArea,
        mesh.vertices.size(), mesh.triangles.size(),
        double(bmin.x), double(bmin.y), double(bmin.z),
        double(bmax.x), double(bmax.y), double(bmax.z));
    return true;
}

/// Returns a compact JSON geometry brief (bounding box, volume, surface area,
/// topology counts) for the currently primary-selected object.
bool MirEngineGetSelectedObjectMetrics(
    void* viewport,
    char* outJson,
    size_t outCapacity
)
{
    if (!outJson || outCapacity == 0)
        return false;

    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene)
    {
        std::snprintf(outJson, outCapacity, "{\"hasGeometry\":false}");
        return true;
    }

    const mir4d::ObjectId selected = native->runtime->state().selection.primary();
    return writeObjectMetrics(native->scene->find(selected), outJson, outCapacity);
}

/// Returns the same JSON geometry brief as MirEngineGetSelectedObjectMetrics
/// but for an arbitrary object addressed by id. Used by the multi-selection
/// inspector to show per-item metrics.
bool MirEngineGetObjectMetricsById(
    void* viewport,
    std::uint64_t objectId,
    char* outJson,
    size_t outCapacity)
{
    if (!outJson || outCapacity == 0)
        return false;

    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene)
    {
        std::snprintf(outJson, outCapacity, "{\"hasGeometry\":false}");
        return true;
    }

    return writeObjectMetrics(native->scene->find(objectId), outJson, outCapacity);
}

double MirEngineGetSelectionFaceArea(void* viewport)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene)
        return 0.0;

    const auto& sel = native->runtime->state().selection;
    if (sel.kind() != mir::PickKind::Face)
        return 0.0;

    const auto node = native->scene->find(sel.primary());
    if (!node || !node->model() || node->model()->mesh().empty())
        return 0.0;

    const auto& mesh = node->model()->mesh();
    const std::uint64_t faceId = sel.elementId();
    const mir::Transform transform = node->transform();

    double area = 0.0;
    bool found = false;
    for (const auto& tri : mesh.triangles)
    {
        if (tri.sourceFaceId != faceId)
            continue;
        found = true;
        const mir::Point3 a = transform.transformPoint(mesh.vertices[tri.a]);
        const mir::Point3 b = transform.transformPoint(mesh.vertices[tri.b]);
        const mir::Point3 c = transform.transformPoint(mesh.vertices[tri.c]);
        const mir::Vector3 e1 = b - a;
        const mir::Vector3 e2 = c - a;
        area += 0.5 * mir::Vector3::cross(e1, e2).length();
    }
    return found ? area : 0.0;
}

/// Exact edge length for a mesh edge element (elementId = ti*3 + k). Uses the
/// retained B-Rep source edge when available (straight or curved), falling back
/// to 0.0 so callers can use the cheaper chord length. The B-Rep length is in
/// the node's local space; it is scaled by the node's average scale to match
/// the world-space chord length used elsewhere.
static double exactEdgeLength(const mir::Scene::NodePtr& node, std::uint64_t elementId)
{
    if (!node || !node->model() || node->model()->mesh().empty())
        return 0.0;

    const auto& mesh = node->model()->mesh();
    const std::size_t ti = static_cast<std::size_t>(elementId / 3);
    const int k = static_cast<int>(elementId % 3);
    if (ti >= mesh.triangles.size())
        return 0.0;

    const std::uint64_t sid = mesh.triangles[ti].sourceEdgeId[k];
    if (sid == mir::kInvalidSourceEdge)
        return 0.0;

    const auto& brep = node->brep();
    if (!brep)
        return 0.0;

    mir::BRepEdgeHandle eh;
    eh.index = sid;
    mir::BRepAdaptor_Curve curve(*brep, eh);
    if (!curve.isBound())
        return 0.0;

    const double local = curve.lengthEstimate();
    if (local <= 0.0)
        return 0.0;

    const mir::Transform transform = node->transform();
    const double scale = (transform.scale.x + transform.scale.y + transform.scale.z) / 3.0;
    return local * scale;
}

double MirEngineGetSelectionEdgeLength(void* viewport)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene)
        return 0.0;

    const auto& sel = native->runtime->state().selection;
    if (sel.kind() != mir::PickKind::Edge)
        return 0.0;

    const auto node = native->scene->find(sel.primary());
    if (!node || !node->model() || node->model()->mesh().empty())
        return 0.0;

    // Prefer the exact B-Rep edge length (correct for straight and curved edges)
    // when the mesh edge carries B-Rep provenance and the source B-Rep is kept.
    const double exact = exactEdgeLength(node, sel.elementId());
    if (exact > 0.0)
        return exact;

    const auto& mesh = node->model()->mesh();
    const std::uint64_t elementId = sel.elementId();
    const std::size_t ti = static_cast<std::size_t>(elementId / 3);
    const int e = static_cast<int>(elementId % 3);
    if (ti >= mesh.triangles.size())
        return 0.0;

    const auto& tri = mesh.triangles[ti];
    const std::size_t ends[2] = {
        (e == 0) ? tri.a : (e == 1) ? tri.b : tri.c,
        (e == 0) ? tri.b : (e == 1) ? tri.c : tri.a};
    const mir::Transform transform = node->transform();
    const mir::Point3 a = transform.transformPoint(mesh.vertices[ends[0]]);
    const mir::Point3 b = transform.transformPoint(mesh.vertices[ends[1]]);
    return (b - a).length();
}

// Returns the stable source B-Rep edge id of the currently selected edge, or
// kInvalidSourceEdge when the selection is not an edge / has no provenance.
// The element id encodes the triangle and local edge (elementId = ti*3 + k),
// so the source edge is read directly from the tessellated mesh.
std::uint64_t MirEngineGetSelectionEdgeSourceId(void* viewport)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene)
        return mir::kInvalidSourceEdge;

    const auto& sel = native->runtime->state().selection;
    if (sel.kind() != mir::PickKind::Edge)
        return mir::kInvalidSourceEdge;

    const auto node = native->scene->find(sel.primary());
    if (!node || !node->model() || node->model()->mesh().empty())
        return mir::kInvalidSourceEdge;

    const auto& mesh = node->model()->mesh();
    const std::uint64_t elementId = sel.elementId();
    const std::size_t ti = static_cast<std::size_t>(elementId / 3);
    const int e = static_cast<int>(elementId % 3);
    if (ti >= mesh.triangles.size())
        return mir::kInvalidSourceEdge;

    return mesh.triangles[ti].sourceEdgeId[e];
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

void MirEngineSelectObject(
    void* viewport,
    uint64_t objectId
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return;

    native->runtime->state().selection.select(mir4d::ObjectId(objectId), false);
}

bool MirEnginePickWorldPoint(
    void* viewport,
    double nx, double ny,
    double* outX, double* outY, double* outZ,
    uint64_t* outObjectId
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene)
        return false;

    auto* rt = native->runtime.get();
    const double w = static_cast<double>(rt->state().width);
    const double h = static_cast<double>(rt->state().height);
    if (w <= 0 || h <= 0)
        return false;

    // Normalized (-1..1, screen-centred, y up) → pixel coordinates.
    const double px = (nx * 0.5 + 0.5) * w;
    const double py = (0.5 - ny * 0.5) * h;

    const auto ray = mir::RayPicker::buildRay(
        rt->state().camera,
        static_cast<mir::Scalar>(px),
        static_cast<mir::Scalar>(py),
        rt->state().width,
        rt->state().height);
    const auto result = rt->pick(
        static_cast<mir::Scalar>(px),
        static_cast<mir::Scalar>(py));
    if (!result.hit())
        return false;

    const auto point = ray.origin + ray.direction * result.distance;
    if (outX) *outX = static_cast<double>(point.x);
    if (outY) *outY = static_cast<double>(point.y);
    if (outZ) *outZ = static_cast<double>(point.z);
    if (outObjectId) *outObjectId = static_cast<std::uint64_t>(result.objectId);
    return true;
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


bool MirEngineBeginDeformSelected(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return false;

    native->runtime->beginDeformSelected();
    return true;
}


bool MirEngineEndDeformSelected(
    void* viewport
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->runtime)
        return false;

    native->runtime->endDeformSelected();
    return true;
}


// ------------------------------------------------------------
// Hand Grab — Vertical Slice v0.1
// ------------------------------------------------------------

bool MirEnginePickHandRay(
    void* viewport,
    double ox, double oy, double oz,
    double dx, double dy, double dz,
    uint64_t* outObjectId,
    double* outDistance
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !native->scene)
        return false;

    auto* rt = native->runtime.get();
    const mir::Vector3 dir{dx, dy, dz};
    if (dir.isZero())
        return false;

    const auto result = rt->pickWorldRay(
        mir::Point3{ox, oy, oz},
        dir.normalized());
    if (!result.hit())
        return false;

    if (outObjectId)
        *outObjectId = static_cast<std::uint64_t>(result.objectId);
    if (outDistance)
        *outDistance = static_cast<double>(result.distance);
    return true;
}

void MirEngineBeginGrab(
    void* viewport,
    uint64_t objectId
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->beginHandGrab(mir4d::ObjectId(objectId));
}

bool MirEnginePreviewGrab(
    void* viewport,
    uint64_t objectId,
    MirTransform transform
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return false;

    (void)objectId; // runtime tracks the armed grab id; parameter kept for ABI symmetry
    native->runtime->previewHandGrab(fromMirTransform(transform));
    return true;
}

bool MirEngineCommitGrab(
    void* viewport,
    uint64_t objectId
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return false;

    (void)objectId;
    native->runtime->commitHandGrab();
    return true;
}

void MirEngineCancelGrab(
    void* viewport
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->cancelHandGrab();
}

bool MirEngineGetObjectTransform(
    void* viewport,
    uint64_t objectId,
    MirTransform* outTransform
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !outTransform)
        return false;

    *outTransform = toMirTransform(
        native->runtime->objectTransform(mir4d::ObjectId(objectId)));
    return true;
}

void MirEngineSetHandHover(
    void* viewport,
    uint64_t objectId
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    native->runtime->setHandHover(mir4d::ObjectId(objectId));
}

void MirEngineSetHandSkeleton(
    void* viewport,
    int32_t mode,
    int32_t handCount,
    const double* positions,
    const double* confidence,
    const int32_t* handedness,
    const double* pinch,
    const int32_t* gesture
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;

    // Invalid arguments (or mode off) clear the overlay rather than draw stale data.
    if (mode <= 0 || handCount <= 0 || !positions || !confidence ||
        !handedness || !pinch)
    {
        native->runtime->clearHandSkeleton();
        return;
    }

    MirEngine::Rendering::HandSkeletonRenderData data{};
    data.mode = static_cast<int>(mode);
    const int hands = std::min(static_cast<int>(handCount),
                               MirEngine::Rendering::HandSkeletonRenderData::kMaxHands);
    data.handCount = hands;
    for (int h = 0; h < hands; ++h)
    {
        data.handedness[h] = static_cast<int>(handedness[h]);
        data.pinch[h] = static_cast<float>(pinch[h]);
        data.gesture[h] = gesture ? static_cast<int>(gesture[h]) : 0;
        for (int j = 0; j < MirEngine::Rendering::HandSkeletonRenderData::kMaxJoints; ++j)
        {
            const std::size_t src = static_cast<std::size_t>(h) * 21 + static_cast<std::size_t>(j);
            data.positions[h][j * 3 + 0] = static_cast<float>(positions[src * 3 + 0]);
            data.positions[h][j * 3 + 1] = static_cast<float>(positions[src * 3 + 1]);
            data.positions[h][j * 3 + 2] = static_cast<float>(positions[src * 3 + 2]);
            data.confidence[h][j] = static_cast<float>(confidence[src]);
        }
    }
    native->runtime->setHandSkeleton(data);
}

void MirEngineSetHandSkeletonStyle(
    void* viewport,
    float leftR, float leftG, float leftB,
    float rightR, float rightG, float rightB,
    float jointSize, float tipSize, float wristSize,
    float alpha, int32_t depthTest
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;
    MirEngine::Rendering::HandSkeletonStyle style{};
    style.leftColor[0] = leftR;  style.leftColor[1] = leftG;  style.leftColor[2] = leftB;
    style.rightColor[0] = rightR; style.rightColor[1] = rightG; style.rightColor[2] = rightB;
    style.jointSize = jointSize;
    style.tipSize = tipSize;
    style.wristSize = wristSize;
    style.alpha = alpha;
    style.depthTest = depthTest != 0;
    native->runtime->setHandSkeletonStyle(style);
}

void MirEngineSetHandSkeletonTopology(
    void* viewport,
    int32_t boneCount,
    const int32_t* bones
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || boneCount <= 0 || !bones)
        return;
    std::vector<std::pair<int, int>> topology;
    topology.reserve(static_cast<std::size_t>(boneCount));
    for (int i = 0; i < boneCount; ++i)
        topology.emplace_back(static_cast<int>(bones[i * 2 + 0]),
                              static_cast<int>(bones[i * 2 + 1]));
    native->runtime->setHandSkeletonTopology(topology);
}

void MirEngineClearHandSkeleton(void* viewport)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime)
        return;
    native->runtime->clearHandSkeleton();
}

bool MirEngineGetCameraEye(
    void* viewport,
    double* outX,
    double* outY,
    double* outZ
)
{
    auto* native = asViewport(viewport);
    if (!native || !native->runtime || !outX || !outY || !outZ)
        return false;

    const auto eye = native->runtime->state().camera.position();
    *outX = static_cast<double>(eye.x);
    *outY = static_cast<double>(eye.y);
    *outZ = static_cast<double>(eye.z);
    return true;
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

    // Retain the exact B-Rep source on the node so downstream queries (exact
    // edge length, STEP round-trip) can use it. The tessellated mesh alone
    // only approximates the geometry.
    if (auto node = native->scene->find(inserted.objectId))
        node->setBrep(std::make_shared<mir::BRepModel>(brep));

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


bool MirEngineImportStep(
    void* viewport,
    const char* path
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->scene)
    {
        setLastError(native, "MIR4D STEP import: viewport is not ready");
        return false;
    }

    if (path == nullptr || *path == '\0')
    {
        setLastError(native, "MIR4D STEP import: path is empty");
        return false;
    }

    const mir::io::ImportResult imported =
        mir::io::step::StepImporter{}.importFile(path);

    if (!imported.ok())
    {
        setLastError(
            native,
            imported.error.empty()
                ? "MIR4D STEP import: parsing failed"
                : imported.error.c_str());
        return false;
    }

    auto model = std::make_shared<mir::Model>();
    model->setMesh(*imported.mesh);

    const auto node = native->scene->createNode(std::move(model));
    if (!node)
    {
        setLastError(native, "MIR4D STEP import: failed to add mesh to scene");
        return false;
    }

    setLastError(native, nullptr);
    return true;
}


bool MirEngineImportStepBRep(
    void* viewport,
    const char* path
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->scene)
    {
        setLastError(native, "MIR4D STEP B-Rep import: viewport is not ready");
        return false;
    }

    if (path == nullptr || *path == '\0')
    {
        setLastError(native, "MIR4D STEP B-Rep import: path is empty");
        return false;
    }

    std::string error;
    std::shared_ptr<mir::BRepModel> brep =
        mir::io::step::BRepStepBridge::read(path, error);

    if (!brep)
    {
        setLastError(
            native,
            error.empty()
                ? "MIR4D STEP B-Rep import: parsing failed"
                : error.c_str());
        return false;
    }

    if (brep->rootSolids().empty())
    {
        setLastError(native, "MIR4D STEP B-Rep import: no B-Rep solids found");
        return false;
    }

    const mir::TriangleMesh3 mesh =
        mir::BRepTessellator::tessellateModel(*brep);

    if (mesh.vertices.empty() || mesh.triangles.empty() || !mesh.isValid())
    {
        setLastError(native, "MIR4D STEP B-Rep import: tessellation failed");
        return false;
    }

    auto model = std::make_shared<mir::Model>();
    model->setMesh(mesh);

    const auto node = native->scene->createNode(std::move(model));
    if (!node)
    {
        setLastError(native, "MIR4D STEP B-Rep import: failed to add mesh to scene");
        return false;
    }

    node->setBrep(brep);

    setLastError(native, nullptr);
    return true;
}


bool MirEngineExportStep(
    void* viewport,
    const char* path,
    bool selectionOnly
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->scene)
    {
        setLastError(native, "MIR4D STEP export: viewport is not ready");
        return false;
    }

    if (path == nullptr || *path == '\0')
    {
        setLastError(native, "MIR4D STEP export: path is empty");
        return false;
    }

    mir4d::Document document{"MIR4D Viewport STEP Export"};
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
                "MIR4D STEP export: failed to stage scene object");
            return false;
        }
    }

    const mir::io::ExportResult result =
        mir::io::step::StepExporter{}.exportTo(
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


bool MirEngineExportStepBRep(
    void* viewport,
    const char* path,
    bool selectionOnly
)
{
    auto* native =
        asViewport(viewport);

    if (!native || !native->scene)
    {
        setLastError(native, "MIR4D STEP B-Rep export: viewport is not ready");
        return false;
    }

    if (path == nullptr || *path == '\0')
    {
        setLastError(native, "MIR4D STEP B-Rep export: path is empty");
        return false;
    }

    std::vector<std::shared_ptr<mir::BRepModel>> sources;
    std::vector<mir4d::ObjectId> selection;

    if (selectionOnly && native->runtime)
        selection = native->runtime->state().selection.ids();

    for (const auto& node : native->scene->nodes())
    {
        if (!node)
            continue;
        if (!node->brep())
            continue;
        if (selectionOnly)
        {
            bool included = false;
            for (mir4d::ObjectId selected : selection)
                if (selected == node->id())
                {
                    included = true;
                    break;
                }
            if (!included)
                continue;
        }
        sources.push_back(node->brep());
    }

    if (sources.empty())
    {
        setLastError(native, "MIR4D STEP B-Rep export: no exact B-Rep sources in scene");
        return false;
    }

    std::shared_ptr<mir::BRepModel> merged = mir::mergeBRepModels(sources);

    std::string error;
    if (!mir::io::step::BRepStepBridge::write(path, *merged, error))
    {
        setLastError(
            native,
            error.empty()
                ? "MIR4D STEP B-Rep export: write failed"
                : error.c_str());
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

uint32_t MirEngineSketchAddArc(void* doc, float cx, float cy, float r, float startAngle, float endAngle)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return 0;
    return d->geometry().addArc({cx, cy}, r, startAngle, endAngle);
}

uint32_t MirEngineSketchAddSpline(void* doc, const float* xs, const float* ys, uint32_t count, bool closed)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d || !xs || !ys || count < 2) return 0;
    std::vector<mir::SketchPoint2D> pts(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        pts[i].x = static_cast<double>(xs[i]);
        pts[i].y = static_cast<double>(ys[i]);
    }
    return d->geometry().addSpline(std::move(pts), closed);
}

uint32_t MirEngineSketchSplineCount(void* doc)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return 0;
    uint32_t n = 0;
    for (const auto& g : d->geometry().all())
    {
        if (std::holds_alternative<mir::SketchSpline2D>(g)) ++n;
    }
    return n;
}

bool MirEngineSketchSplineAt(void* doc, uint32_t index, float* xs, float* ys, uint32_t* count, bool* closed)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d || !count) return false;
    const auto& all = d->geometry().all();
    if (index >= all.size()) return false;
    const auto* s = std::get_if<mir::SketchSpline2D>(&all[index]);
    if (!s) return false;
    const uint32_t available = static_cast<uint32_t>(s->controlPoints.size());
    if (closed) *closed = s->closed;
    if (xs && ys)
    {
        const uint32_t toCopy = std::min(*count, available);
        for (uint32_t i = 0; i < toCopy; ++i)
        {
            xs[i] = static_cast<float>(s->controlPoints[i].x);
            ys[i] = static_cast<float>(s->controlPoints[i].y);
        }
    }
    *count = available;
    return true;
}

uint32_t MirEngineSketchGeometryCount(void* doc)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return 0;
    return static_cast<uint32_t>(d->geometry().all().size());
}

int MirEngineSketchGeometryTypeAt(void* doc, uint32_t index)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return -1;
    const auto& all = d->geometry().all();
    if (index >= all.size()) return -1;
    return static_cast<int>(all[index].index());
}

bool MirEngineSketchLineAt(void* doc, uint32_t index, float* x1, float* y1, float* x2, float* y2)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return false;
    const auto& all = d->geometry().all();
    if (index >= all.size()) return false;
    if (const auto* l = std::get_if<mir::SketchLine2D>(&all[index]))
    {
        if (x1) *x1 = static_cast<float>(l->start.x);
        if (y1) *y1 = static_cast<float>(l->start.y);
        if (x2) *x2 = static_cast<float>(l->end.x);
        if (y2) *y2 = static_cast<float>(l->end.y);
        return true;
    }
    return false;
}

bool MirEngineSketchArcAt(void* doc, uint32_t index, float* cx, float* cy, float* r, float* sa, float* ea)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return false;
    const auto& all = d->geometry().all();
    if (index >= all.size()) return false;
    if (const auto* a = std::get_if<mir::SketchArc2D>(&all[index]))
    {
        if (cx) *cx = static_cast<float>(a->center.x);
        if (cy) *cy = static_cast<float>(a->center.y);
        if (r) *r = static_cast<float>(a->radius);
        if (sa) *sa = static_cast<float>(a->startAngle);
        if (ea) *ea = static_cast<float>(a->endAngle);
        return true;
    }
    return false;
}

bool MirEngineSketchCircleAt(void* doc, uint32_t index, float* cx, float* cy, float* r)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return false;
    const auto& all = d->geometry().all();
    if (index >= all.size()) return false;
    if (const auto* cc = std::get_if<mir::SketchCircle2D>(&all[index]))
    {
        if (cx) *cx = static_cast<float>(cc->center.x);
        if (cy) *cy = static_cast<float>(cc->center.y);
        if (r) *r = static_cast<float>(cc->radius);
        return true;
    }
    return false;
}

bool MirEngineSketchRemoveGeometry(void* doc, uint32_t id)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return false;
    return d->geometry().remove(id);
}

void MirEngineSketchClear(void* doc)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (d) d->clear();
}

void MirEngineSketchSetPlane(void* doc, uint32_t planeId,
                             float ox, float oy, float oz,
                             float nx, float ny, float nz,
                             float xx, float xy, float xz,
                             float yx, float yy, float yz)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return;
    mir::Matrix4 m = mir::Matrix4::identity();
    m(0, 0) = xx; m(1, 0) = xy; m(2, 0) = xz;
    m(0, 1) = yx; m(1, 1) = yy; m(2, 1) = yz;
    m(0, 2) = nx; m(1, 2) = ny; m(2, 2) = nz;
    m(0, 3) = ox; m(1, 3) = oy; m(2, 3) = oz;
    d->setPlane(planeId, m);
}

uint32_t MirEngineSketchGeometryIdAt(void* doc, uint32_t index)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return 0;
    const auto& all = d->geometry().all();
    if (index >= all.size()) return 0;
    return std::visit([](const auto& g) { return g.id; }, all[index]);
}

bool MirEngineSketchRemoveConstraint(void* doc, uint32_t id)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return false;
    return d->constraints().remove(id);
}

uint32_t MirEngineSketchConstraintCount(void* doc)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return 0;
    return static_cast<uint32_t>(d->constraints().all().size());
}

bool MirEngineSketchConstraintAt(void* doc, uint32_t index, int32_t* type, uint32_t* g1, uint32_t* g2, double* value)
{
    auto* d = reinterpret_cast<mir::SketchDocument*>(doc);
    if (!d) return false;
    const auto& all = d->constraints().all();
    if (index >= all.size()) return false;
    const auto& c = all[index];
    if (type) *type = static_cast<int32_t>(c.type);
    if (g1) *g1 = c.firstGeometry;
    if (g2) *g2 = c.secondGeometry;
    if (value) *value = c.value;
    return true;
}

// ------------------------------------------------------------
// VFX — собственная подсистема визуальных эффектов
// ------------------------------------------------------------

void MirEngineVFXTrigger(int kind)
{
    using namespace MirEngine::VFX;
    if (kind < 0 || kind >= static_cast<int>(EffectKind::Count))
        return;
    EffectSystem::instance().trigger(static_cast<EffectKind>(kind));
}

void MirEngineVFXUpdate(float dt)
{
    MirEngine::VFX::EffectSystem::instance().update(dt);
}

void MirEngineVFXRender()
{
    MirEngine::VFX::EffectSystem::instance().render();
}

void MirEngineVFXReset()
{
    MirEngine::VFX::EffectSystem::instance().reset();
}

}
