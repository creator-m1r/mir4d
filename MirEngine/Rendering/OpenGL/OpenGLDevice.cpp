
#include "OpenGLDevice.h"
#include "OpenGLContext.h"
#include "OpenGLShader.h"
#include "OpenGLVertexArray.h"

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

#include <iostream>

namespace MirEngine::Rendering {

OpenGLDevice::OpenGLDevice(OpenGLContext* context)
    : m_context(context)
{
}

OpenGLDevice::~OpenGLDevice()
{

    ResetDefaultVertexArray();
}

bool OpenGLDevice::initialize()
{
    if (!m_context)
    {
        return false;
    }

    m_context->makeCurrent();

    BindDefaultVertexArray();
    m_state.initialize();

    const Size2D size = m_context->size();
    m_state.setViewport(size.width, size.height);

    return true;
}

void OpenGLDevice::clear(const ColorRGBA& color,
                         float depth,
                         int stencil,
                         ClearFlags flags)
{
    if (!m_context)
        return;

    m_context->makeCurrent();
    m_state.clear(color, depth, stencil, flags);
}

void OpenGLDevice::draw(const RenderCommand& command)
{
    if (!m_context)
        return;

    m_context->makeCurrent();

    const auto meshIt = m_meshes.find(command.mesh);
    if (meshIt == m_meshes.end())
    {
        std::cerr << "[OpenGLDevice] Mesh handle not found: "
                  << command.mesh << "\n";
        return;
    }
    const auto materialIt = m_materials.find(command.material);
    if (materialIt == m_materials.end())
    {
        std::cerr << "[OpenGLDevice] Material handle not found: "
                  << command.material << "\n";
        return;
    }

    const auto& mesh = meshIt->second;
    const auto& shader = materialIt->second;

    shader->bind();
    shader->setMatrix("u_model", command.modelMatrix);

    shader->setMatrix("u_view", m_viewMatrix);
    shader->setMatrix("u_projection", m_projectionMatrix);

    m_state.setDepthTest(command.state.depthTest);
    m_state.setBlend(command.state.blend);
    m_state.setWireframe(command.state.wireframe);
    m_state.setLineWidth(command.state.lineWidth);

    mesh->bind();

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

    if (mesh->hasIndexBuffer())
    {
        const GLsizei count = (command.indexCount != 0)
            ? static_cast<GLsizei>(command.indexCount)
            : static_cast<GLsizei>(mesh->getElementCount());
        glDrawElements(primitive, count, GL_UNSIGNED_INT, nullptr);
    }
    else
    {
        const GLsizei count = (command.indexCount != 0)
            ? static_cast<GLsizei>(command.indexCount)
            : static_cast<GLsizei>(mesh->getElementCount());
        glDrawArrays(primitive, static_cast<GLint>(command.firstIndex), count);
    }

    mesh->unbind();
    shader->unbind();
}

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
    if (mesh)
    {
        m_meshes[handle] = std::move(mesh);
    }
}

void OpenGLDevice::registerMaterial(MaterialHandle handle, std::shared_ptr<Shader> shader)
{
    if (shader)
    {
        m_materials[handle] = std::move(shader);
    }
}

void OpenGLDevice::present()
{
    if (!m_context)
        return;

    m_state.endFrame();
    m_context->swapBuffers();
}

void OpenGLDevice::setViewportSize(std::uint32_t width, std::uint32_t height)
{
    if (!m_context)
        return;

    m_context->resize({width, height});
    m_state.setViewport(width, height);
}

void OpenGLDevice::setViewMatrix(const Matrix4Raw& viewMatrix)
{
    m_viewMatrix = viewMatrix;
}

void OpenGLDevice::setProjectionMatrix(const Matrix4Raw& projMatrix)
{
    m_projectionMatrix = projMatrix;
}

void OpenGLDevice::beginFrame()
{
    if (!m_context)
        return;

    m_context->makeCurrent();
    m_state.beginFrame();
}

void OpenGLDevice::endFrame()
{
    present();
}

std::unique_ptr<RenderDevice> CreateRenderDevice(OpenGLContext* context)
{
    if (!context)
    {
        return nullptr;
    }

    auto device = std::make_unique<OpenGLDevice>(context);
    if (!device->initialize())
    {
        return nullptr;
    }
    return device;
}

}