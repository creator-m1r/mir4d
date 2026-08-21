#include "OpenGLRenderer.h"

#include <chrono>
#include <cstdio>

#include "OpenGLContext.h"
#include "OpenGLDebug.h"
#include "OpenGLDevice.h"

#include "MirEngine/Core/Identity/ObjectId.hpp"

#include "../Core/RenderDevice.h"
#include "../Core/RenderContext.h"

#include "MirEngine/Geometry/Scene/Scene.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include <cstdio>

namespace {

void dbgTimestamp(const char* what)
{
    const auto now = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now.time_since_epoch())
                        .count();
    std::fprintf(stderr, "[MIR4D-DBG %lldms] %s\n",
                 static_cast<long long>(ms), what);
}

}

namespace MirEngine::Rendering
{

namespace
{

Matrix4Raw makeCameraRelativeView(const Matrix4Raw& view) noexcept
{
    Matrix4Raw result = view;
    result[12] = 0.0f;
    result[13] = 0.0f;
    result[14] = 0.0f;
    return result;
}
}

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

    dbgTimestamp("initialize: makeCurrent");
    m_context->makeCurrent();
    dbgTimestamp("initialize: device");
    m_device = std::make_unique<OpenGLDevice>(m_context);
    if (!m_device->initialize())
    {
        m_device.reset();
        return false;
    }
    dbgTimestamp("initialize: device ok");

    m_gridPass = std::make_unique<GridPass>();
    if (!m_gridPass->initialize(*m_device))
    {
        std::cerr << "[OpenGLRenderer] GridPass initialization failed; continuing without grid.\n";
        m_gridPass.reset();
    }
    dbgTimestamp("initialize: gridPass ok");

    m_geometryPass = std::make_unique<GeometryPass>(m_shaderLibrary);
    if (!m_geometryPass->initialize(*m_device))
    {
        std::cerr << "[OpenGLRenderer] GeometryPass initialization failed; continuing without geometry.\n";
        m_geometryPass.reset();
    }
    dbgTimestamp("initialize: geometryPass ok");

    m_planePass = std::make_unique<PlanePass>();
    if (!m_planePass->initialize(*m_device))
    {
        std::cerr << "[OpenGLRenderer] PlanePass initialization failed; continuing without planes.\n";
        m_planePass.reset();
    }
    dbgTimestamp("initialize: planePass ok");

    m_sketchPass = std::make_unique<SketchPass>();
    if (!m_sketchPass->initialize(*m_device))
    {
        std::cerr << "[OpenGLRenderer] SketchPass initialization failed; continuing without sketch overlay.\n";
        m_sketchPass.reset();
    }
    dbgTimestamp("initialize: sketchPass ok");

    m_handSkeletonPass = std::make_unique<HandSkeletonPass>();
    if (!m_handSkeletonPass->initialize(*m_device))
    {
        std::cerr << "[OpenGLRenderer] HandSkeletonPass initialization failed; continuing without skeleton overlay.\n";
        m_handSkeletonPass.reset();
    }
    dbgTimestamp("initialize: handSkeletonPass ok");

    dbgTimestamp("initialize: resetErrors");
    OpenGLDebug::resetErrors();
    dbgTimestamp("initialize: logReport");
    OpenGLDebug::logReport();
    dbgTimestamp("initialize: logReport done");
    OpenGLDebug::enableDebugOutput();
    dbgTimestamp("initialize: debug output done");

    m_initialized = true;
    dbgTimestamp("initialize: DONE");
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

    if (std::getenv("MIR4D_SCREENSHOT") != nullptr)
    {
        captureDiagnosticFrame(context, scene, *m_device);
        return;
    }

    m_device->beginFrame();
    m_device->clear(ColorRGBA{0.055f, 0.065f, 0.085f, 1.0f});

    const Matrix4Raw cameraRelativeView = makeCameraRelativeView(context.viewMatrix);
    m_device->setViewMatrix(cameraRelativeView);
    m_device->setProjectionMatrix(context.projectionMatrix);

    if (m_gridPass && m_gridPass->isInitialized())
        m_gridPass->execute(context, scene, *m_device);

    if (!m_planes.empty())
        context.planes = m_planes;
    else
        context.planes.clear();

    context.cursorNDC[0] = m_cursorNDC[0];
    context.cursorNDC[1] = m_cursorNDC[1];
    context.cursorActive = m_cursorActive;

    if (m_planePass && m_planePass->isInitialized())
        m_planePass->execute(context, scene, *m_device);

    if (!m_sketches.empty())
        context.sketches = m_sketches;
    else
        context.sketches.clear();

    if (m_sketchPass && m_sketchPass->isInitialized())
        m_sketchPass->execute(context, scene, *m_device);

    if (m_geometryPass)
        m_geometryPass->execute(context, scene, *m_device);

    if (m_handSkeletonPass && m_handSkeletonPass->isInitialized())
        m_handSkeletonPass->execute(context, scene, *m_device);

    m_device->endFrame();
}

void OpenGLRenderer::captureDiagnosticFrame(RenderContext& context,
                                            mir::Scene& scene,
                                            RenderDevice& device)
{
    static bool s_captured = false;
    if (s_captured)
        return;
    s_captured = true;

    const int w = std::max(static_cast<int>(context.viewportWidth), 256);
    const int h = std::max(static_cast<int>(context.viewportHeight), 256);

    GLuint fbo = 0;
    GLuint color = 0;
    GLuint depth = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, color, 0);

    glGenRenderbuffers(1, &depth);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depth);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "[DIAG] FBO incomplete: 0x" << std::hex << status
                  << std::dec << "\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &color);
        glDeleteRenderbuffers(1, &depth);
        return;
    }

    glViewport(0, 0, w, h);
    glClearColor(0.055f, 0.065f, 0.085f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const Matrix4Raw cameraRelativeView =
        makeCameraRelativeView(context.viewMatrix);
    device.setViewMatrix(cameraRelativeView);
    device.setProjectionMatrix(context.projectionMatrix);

    if (m_gridPass && m_gridPass->isInitialized())
        m_gridPass->execute(context, scene, device);

    if (!m_planes.empty())
        context.planes = m_planes;
    else
        context.planes.clear();

    context.cursorNDC[0] = m_cursorNDC[0];
    context.cursorNDC[1] = m_cursorNDC[1];
    context.cursorActive = m_cursorActive;

    if (m_planePass && m_planePass->isInitialized())
        m_planePass->execute(context, scene, device);

    if (!m_sketches.empty())
        context.sketches = m_sketches;
    else
        context.sketches.clear();

    if (m_sketchPass && m_sketchPass->isInitialized())
        m_sketchPass->execute(context, scene, device);

    if (m_geometryPass)
        m_geometryPass->execute(context, scene, device);

    if (m_handSkeletonPass && m_handSkeletonPass->isInitialized())
        m_handSkeletonPass->execute(context, scene, device);

    std::vector<unsigned char> px(static_cast<std::size_t>(w) * h * 3);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());

    FILE* f = std::fopen("/tmp/mir4d_frame.ppm", "wb");
    if (f)
    {
        std::fprintf(f, "P6\n%d %d\n255\n", w, h);
        for (int y = h - 1; y >= 0; --y)
        {
            std::fwrite(px.data() + static_cast<std::size_t>(y) * w * 3, 1,
                        static_cast<std::size_t>(w) * 3, f);
        }
        std::fclose(f);
        std::cerr << "[DIAG] wrote /tmp/mir4d_frame.ppm " << w << "x" << h
                  << "\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &color);
    glDeleteRenderbuffers(1, &depth);
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

}