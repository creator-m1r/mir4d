// MirEngine/Rendering/OpenGL/OpenGLDevice.cpp
// =================================================================================
// Реализация OpenGLDevice.
// =================================================================================

#include "OpenGLDevice.h"
#include "OpenGLContext.h"

// Реализация draw через кеш мешей и материалов.
#include "OpenGLShader.h"

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

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
// Рисование (через кеш мешей и материалов)
// --------------------------------------------------------------------------
void OpenGLDevice::draw(const RenderCommand& command)
{
    if (!m_context) return;

    m_context->makeCurrent();

    auto meshIt = m_meshes.find(command.mesh);
    if (meshIt == m_meshes.end()) {
        std::cerr << "[OpenGLDevice] Mesh handle not found: " << command.mesh << "\n";
        return;
    }
    auto materialIt = m_materials.find(command.material);
    if (materialIt == m_materials.end()) {
        std::cerr << "[OpenGLDevice] Material handle not found: " << command.material << "\n";
        return;
    }

    auto& mesh = meshIt->second;
    auto& shader = materialIt->second;

    shader->bind();
    shader->setMatrix("u_model", command.modelMatrix);
    shader->setMatrix("u_view", m_viewMatrix);
    shader->setMatrix("u_projection", m_projectionMatrix);

    mesh->bind();

    m_state.setWireframe(command.wireframe);

    const GLenum primitive = [](PrimitiveType type) -> GLenum
    {
        switch (type)
        {
        case PrimitiveType::Points:        return GL_POINTS;
        case PrimitiveType::Lines:         return GL_LINES;
        case PrimitiveType::LineStrip:     return GL_LINE_STRIP;
        case PrimitiveType::LineLoop:      return GL_LINE_LOOP;
        case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
        case PrimitiveType::TriangleFan:   return GL_TRIANGLE_FAN;
        case PrimitiveType::Triangles:
        default:                           return GL_TRIANGLES;
        }
    }(command.primitive);

    const GLsizei count = (command.indexCount != 0)
        ? static_cast<GLsizei>(command.indexCount)
        : static_cast<GLsizei>(mesh->getElementCount());

    glDrawElements(primitive, count, GL_UNSIGNED_INT, nullptr);

    mesh->unbind();
    shader->unbind();
}

// --------------------------------------------------------------------------
// Создание GPU-ресурсов
// --------------------------------------------------------------------------
std::shared_ptr<VertexBuffer> OpenGLDevice::createVertexBuffer()
{
    return std::make_shared<OpenGLVertexBuffer>();
}

std::shared_ptr<IndexBuffer> OpenGLDevice::createIndexBuffer()
{
    return std::make_shared<OpenGLIndexBuffer>();
}

std::shared_ptr<VertexArray> OpenGLDevice::createVertexArray()
{
    return std::make_shared<OpenGLVertexArray>();
}

void OpenGLDevice::registerMesh(MeshHandle handle, std::shared_ptr<VertexArray> mesh)
{
    if (mesh) {
        m_meshes[handle] = std::move(mesh);
    }
}

void OpenGLDevice::registerMaterial(MaterialHandle handle, std::shared_ptr<Shader> shader)
{
    if (shader) {
        m_materials[handle] = std::move(shader);
    }
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