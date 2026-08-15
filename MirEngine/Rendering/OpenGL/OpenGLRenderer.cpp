#include "OpenGLRenderer.h"
#include "OpenGLContext.h"
#include "OpenGLDebug.h"
#include "OpenGLDevice.h"

#include "MirEngine/Core/Identity/ObjectId.hpp"

#include "../Core/RenderDevice.h"
#include "../Core/RenderContext.h"

#include "MirEngine/Geometry/Scene/Scene.hpp"

#include <iostream>
#include <memory>

namespace MirEngine::Rendering
{

namespace
{
// Camera-relative rendering contract (shared with GeometryPass):
//   - u_model carries the world translation already shifted by -cameraPosition
//     (computed in double precision on the CPU, so large CAD coordinates stay
//     numerically stable on the GPU);
//   - u_view must therefore be the rotation-only view matrix; passing the full
//     translated view would subtract the camera position a second time and move
//     every model out of its expected position.
// Matrix4Raw is column-major (element (row, col) lives at row + col * 4), so
// the camera translation column (rows 0..2, column 3) occupies indices 12..14.
Matrix4Raw makeCameraRelativeView(const Matrix4Raw& view) noexcept
{
    Matrix4Raw result = view;
    result[12] = 0.0f;
    result[13] = 0.0f;
    result[14] = 0.0f;
    return result;
}
} // namespace

OpenGLRenderer::OpenGLRenderer(OpenGLContext* context)
    : m_context(context)
    , m_shaderLibrary([] {
        return std::make_shared<OpenGLShader>();
    })
{
}

OpenGLRenderer::~OpenGLRenderer() = default;

bool OpenGLRenderer::initialize()
{
    if (!m_context)
        return false;

    m_context->makeCurrent();
    m_device = std::make_unique<OpenGLDevice>(m_context);
    if (!m_device->initialize())
    {
        m_device.reset();
        return false;
    }

    m_gridPass = std::make_unique<GridPass>();
    if (!m_gridPass->initialize(*m_device))
    {
        std::cerr << "[OpenGLRenderer] GridPass initialization failed; continuing without grid.\n";
        m_gridPass.reset();
    }

    m_geometryPass = std::make_unique<GeometryPass>(m_shaderLibrary);
    if (!m_geometryPass->initialize(*m_device))
    {
        std::cerr << "[OpenGLRenderer] GeometryPass initialization failed; continuing without geometry.\n";
        m_geometryPass.reset();
    }

    // OpenGL diagnostics: context, limits, extensions.
    OpenGLDebug::resetErrors();
    OpenGLDebug::logReport();
    OpenGLDebug::enableDebugOutput();

    m_initialized = true;
    return true;
}

std::string OpenGLRenderer::diagnosticsReport()
{
    if (!m_context)
        return "[GL Diagnostics] No context";
    m_context->makeCurrent();
    return OpenGLDebug::buildReport();
}

void OpenGLRenderer::render(mir::Scene& scene,
                            RenderContext& context)
{
    if (!m_initialized || !m_device || !m_context)
        return;

    m_context->makeCurrent();
    m_device->beginFrame();
    m_device->clear(ColorRGBA{0.055f, 0.065f, 0.085f, 1.0f});

    // GeometryPass performs camera-relative rendering: model translations are
    // shifted by -cameraPosition in double precision.  Keep only the camera
    // rotation in the device view matrix so the camera translation is applied
    // exactly once.
    const Matrix4Raw cameraRelativeView = makeCameraRelativeView(context.viewMatrix);
    m_device->setViewMatrix(cameraRelativeView);
    m_device->setProjectionMatrix(context.projectionMatrix);

    if (m_gridPass && m_gridPass->isInitialized())
        m_gridPass->execute(context, scene, *m_device);

    if (m_geometryPass)
        m_geometryPass->execute(context, scene, *m_device);

    m_device->endFrame();
}

void OpenGLRenderer::setObjectMaterial(std::uint64_t objectId,
                                       std::int32_t materialId) noexcept
{
    if (m_geometryPass)
    {
        m_geometryPass->setObjectMaterial(
            mir4d::ObjectId{objectId},
            static_cast<MaterialId>(materialId));
    }
}

void OpenGLRenderer::resize(std::uint32_t width, std::uint32_t height)
{
    if (!m_device || !m_context)
        return;

    m_context->makeCurrent();
    m_device->setViewportSize(width, height);
}

} // namespace MirEngine::Rendering