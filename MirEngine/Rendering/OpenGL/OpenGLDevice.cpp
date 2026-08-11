// MirEngine/Rendering/OpenGL/OpenGLDevice.cpp
// =================================================================================
// Реализация OpenGLDevice.
// =================================================================================

#include "OpenGLDevice.h"
#include "OpenGLContext.h"

// Временная заглушка draw — полноценная реализация появится
// после подключения VertexArray / Shader / Mesh кеша.
#include <iostream>

namespace MirEngine {
namespace Rendering {

// --------------------------------------------------------------------------
// Конструктор
// --------------------------------------------------------------------------
OpenGLDevice::OpenGLDevice(OpenGLContext* context)
    : m_context(context)
{
}

// --------------------------------------------------------------------------
// Инициализация
// --------------------------------------------------------------------------
bool OpenGLDevice::initialize()
{
    if (!m_context) {
        return false;
    }

    m_context->makeCurrent();
    m_state.initialize();

    const Size2D size = m_context->size();
    m_state.setViewport(size.width, size.height);

    return true;
}

// --------------------------------------------------------------------------
// Очистка
// --------------------------------------------------------------------------
void OpenGLDevice::clear(const ColorRGBA& color,
                         float depth,
                         int stencil,
                         ClearFlags flags)
{
    if (!m_context) return;

    m_context->makeCurrent();
    m_state.clear(color, depth, stencil, flags);
}

// --------------------------------------------------------------------------
// Рисование (пока заглушка)
// --------------------------------------------------------------------------
void OpenGLDevice::draw(const RenderCommand& command)
{
    if (!m_context) return;

    m_context->makeCurrent();

    // TODO: найти Mesh и Material по handle,
    // установить model-матрицу, включить wireframe при необходимости,
    // вызвать glDrawElements / glDrawArrays.

    m_state.setWireframe(command.wireframe);

    // Временная заглушка — просто чтобы код компилировался
    (void)command;
}

// --------------------------------------------------------------------------
// Present
// --------------------------------------------------------------------------
void OpenGLDevice::present()
{
    if (!m_context) return;

    m_state.endFrame();
    m_context->swapBuffers();
}

// --------------------------------------------------------------------------
// Viewport
// --------------------------------------------------------------------------
void OpenGLDevice::setViewportSize(uint32_t width, uint32_t height)
{
    if (!m_context) return;

    m_context->resize({width, height});
    m_state.setViewport(width, height);
}

// --------------------------------------------------------------------------
// Матрицы
// --------------------------------------------------------------------------
void OpenGLDevice::setViewMatrix(const Matrix4Raw& viewMatrix)
{
    m_viewMatrix = viewMatrix;
    // TODO: загрузить в UBO / uniform
}

void OpenGLDevice::setProjectionMatrix(const Matrix4Raw& projMatrix)
{
    m_projectionMatrix = projMatrix;
    // TODO: загрузить в UBO / uniform
}

// --------------------------------------------------------------------------
// Удобные обёртки begin/end frame
// --------------------------------------------------------------------------
void OpenGLDevice::beginFrame()
{
    if (!m_context) return;

    m_context->makeCurrent();
    m_state.beginFrame();
}

void OpenGLDevice::endFrame()
{
    present();
}

// --------------------------------------------------------------------------
// Фабрика (объявлена в RenderDevice.h)
// --------------------------------------------------------------------------
std::unique_ptr<RenderDevice> CreateRenderDevice(OpenGLContext* context)
{
    if (!context) {
        return nullptr;
    }

    auto device = std::make_unique<OpenGLDevice>(context);
    if (!device->initialize()) {
        return nullptr;
    }
    return device;
}

} // namespace Rendering
} // namespace MirEngine