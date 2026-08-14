#include "OpenGLRenderer.h"
#include "OpenGLContext.h"
#include "OpenGLDevice.h"

#include "../Core/RenderDevice.h"
#include "../Core/RenderContext.h"

#include "MirEngine/Geometry/Scene/Scene.hpp"

#include <iostream>
#include <memory>

namespace MirEngine::Rendering
{

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
    if (!m_gridPass->initialize())
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

    m_initialized = true;
    return true;
}

void OpenGLRenderer::render(mir::Scene& scene,
                            RenderContext& context)
{
    if (!m_initialized || !m_device || !m_context)
        return;

    m_context->makeCurrent();
    m_device->beginFrame();
    m_device->clear(ColorRGBA{0.055f, 0.065f, 0.085f, 1.0f});

    if (m_gridPass && m_gridPass->isInitialized())
        m_gridPass->execute(context, scene, *m_device);

    if (m_geometryPass)
        m_geometryPass->execute(context, scene, *m_device);

    m_device->endFrame();
}

void OpenGLRenderer::resize(uint32_t width, uint32_t height)
{
    if (!m_device || !m_context)
        return;

    m_context->makeCurrent();
    m_device->setViewportSize(width, height);
}

} // namespace MirEngine::Rendering
