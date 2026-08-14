// MirEngine/Rendering/OpenGL/OpenGLState.cpp
// =================================================================================
// Реализация управления состоянием OpenGL.
//
// Примечание: используется <OpenGL/gl3.h> (macOS).
// На Windows/Linux в будущем следует заменить на glad / glew / epoxy.
// =================================================================================

#include "OpenGLState.h"

// macOS OpenGL
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
// Заглушка / будущий glad
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

void OpenGLState::initialize()
{
    // Глубина
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0);

    // Отсечение задних граней
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Смешивание (по умолчанию выключено, включается по необходимости)
    glDisable(GL_BLEND);

    // Сглаживание линий (полезно для CAD)
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
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

    if (mask != 0) {
        glClear(mask);
    }
}

void OpenGLState::beginFrame()
{
    // Можно сбрасывать временные состояния, если нужно
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