// MirEngine/Rendering/OpenGL/OpenGLState.cpp
// =================================================================================
// Реализация управления состоянием OpenGL.
// =================================================================================

#include "OpenGLState.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

void OpenGLState::initialize()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    m_wireframe = false;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void OpenGLState::setViewport(uint32_t width, uint32_t height)
{
    m_width  = width;
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

    if ((flags & RenderDevice::ClearFlags::Color) != RenderDevice::ClearFlags{}) {
        glClearColor(color.r, color.g, color.b, color.a);
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if ((flags & RenderDevice::ClearFlags::Depth) != RenderDevice::ClearFlags{}) {
        glClearDepth(static_cast<GLdouble>(depth));
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if ((flags & RenderDevice::ClearFlags::Stencil) != RenderDevice::ClearFlags{}) {
        glClearStencil(stencil);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    if (mask != 0)
        glClear(mask);
}

void OpenGLState::beginFrame()
{
    // Render passes are allowed to change temporary GL state. Restore the
    // canonical CAD viewport state at the beginning of every frame so a grid,
    // selection overlay or wireframe pass can never poison the next frame.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (m_wireframe)
    {
        m_wireframe = false;
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
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

} // namespace Rendering
} // namespace MirEngine
