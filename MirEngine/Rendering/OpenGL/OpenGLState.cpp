// MirEngine/Rendering/OpenGL/OpenGLState.cpp
// =================================================================================
// OpenGL state management implementation.
// =================================================================================

#include "OpenGLState.h"

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine::Rendering {

void OpenGLState::initialize()
{
    if (m_initialized)
        return;

    // Depth testing: CAD scenes rely on correct occlusion.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0);
    m_depthTest = true;

    // Back-face culling with CCW front faces (right-handed winding).
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Blending is off by default; passes enable it explicitly.
    glDisable(GL_BLEND);
    m_blend = false;

    // No multisample toggle here: the default frame buffer owns the
    // multisample configuration (see MacOpenGLContext).
    m_initialized = true;
}

void OpenGLState::setViewport(std::uint32_t width, std::uint32_t height)
{
    m_width = width;
    m_height = height;
    glViewport(0, 0,
               static_cast<GLsizei>(width),
               static_cast<GLsizei>(height));
}

void OpenGLState::clear(const ColorRGBA& color,
                        float depth,
                        int stencil,
                        RenderDevice::ClearFlags flags)
{
    GLbitfield mask = 0;

    if ((flags & RenderDevice::ClearFlags::Color) != RenderDevice::ClearFlags{})
    {
        glClearColor(color.r, color.g, color.b, color.a);
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if ((flags & RenderDevice::ClearFlags::Depth) != RenderDevice::ClearFlags{})
    {
        glClearDepth(static_cast<GLdouble>(depth));
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if ((flags & RenderDevice::ClearFlags::Stencil) != RenderDevice::ClearFlags{})
    {
        glClearStencil(stencil);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    if (mask != 0)
    {
        glClear(mask);
    }
}

void OpenGLState::beginFrame()
{
    // Reset transient state to the defaults before each frame so a pass that
    // forgets to restore its overrides cannot leak into the next pass.
    setWireframe(false);
    setBlend(false);
    setDepthTest(true);
    setLineWidth(1.0f);
    setDepthFunc(RenderDevice::DepthFunc::Less);
}

void OpenGLState::endFrame()
{
    glFlush();
}

void OpenGLState::setWireframe(bool enabled)
{
    if (m_wireframe == enabled)
        return;

    m_wireframe = enabled;
    glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
}

void OpenGLState::setDepthTest(bool enabled)
{
    if (m_depthTest == enabled)
        return;

    m_depthTest = enabled;
    if (enabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void OpenGLState::setBlend(bool enabled)
{
    if (m_blend == enabled)
        return;

    m_blend = enabled;
    if (enabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void OpenGLState::setLineWidth(float width)
{
    const float clamped = width > 0.0f ? width : 1.0f;
    glLineWidth(clamped);
}

void OpenGLState::setDepthFunc(RenderDevice::DepthFunc func)
{
    if (m_depthFunc == func)
        return;

    m_depthFunc = func;
    glDepthFunc(func == RenderDevice::DepthFunc::LessEqual ? GL_LEQUAL : GL_LESS);
}

} // namespace MirEngine::Rendering