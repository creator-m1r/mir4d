
#include "OpenGLVertexBuffer.h"
#include "OpenGLVertexArray.h"
#include <iostream>

namespace MirEngine {
namespace Rendering {

GLenum OpenGLVertexBuffer::usageToGL(BufferUsage usage)
{
    switch (usage) {
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream:  return GL_STREAM_DRAW;
        case BufferUsage::Static:
        default:                   return GL_STATIC_DRAW;
    }
}

OpenGLVertexBuffer::OpenGLVertexBuffer()
{
    glGenBuffers(1, &m_handle);
    if (m_handle == 0) {
        std::cerr << "[OpenGLVertexBuffer] Failed to generate buffer\n";
    }
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
    if (m_handle != 0) {
        glDeleteBuffers(1, &m_handle);
        m_handle = 0;
    }
}

void OpenGLVertexBuffer::uploadVertices(const std::vector<Vertex>& vertices,
                                        BufferUsage usage)
{
    uploadVertices(vertices.data(), vertices.size(), usage);
}

void OpenGLVertexBuffer::uploadVertices(const Vertex* data, size_t count,
                                        BufferUsage usage)
{
    if (!data || count == 0) {
        m_vertexCount = 0;
        return;
    }

    BindDefaultVertexArray();
    bind();

    const GLsizeiptr sizeInBytes = static_cast<GLsizeiptr>(count * sizeof(Vertex));
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, usageToGL(usage));

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "[OpenGLVertexBuffer] glBufferData error: 0x"
                  << std::hex << err << std::dec << "\n";
        m_vertexCount = 0;
    } else {
        m_vertexCount = count;
    }

    unbind();
}

void OpenGLVertexBuffer::upload(const void* data, size_t size,
                                BufferUsage usage)
{
    if (!data || size == 0) {
        m_vertexCount = 0;
        return;
    }

    BindDefaultVertexArray();
    bind();

    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, usageToGL(usage));

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "[OpenGLVertexBuffer] glBufferData error: 0x"
                  << std::hex << err << std::dec << "\n";
        m_vertexCount = 0;
    } else {
        m_vertexCount = size / sizeof(Vertex);
    }

    unbind();
}

void OpenGLVertexBuffer::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_handle);
}

void OpenGLVertexBuffer::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

size_t OpenGLVertexBuffer::getSize() const
{
    return m_vertexCount * sizeof(Vertex);
}

}
}