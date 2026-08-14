// MirEngine/Rendering/OpenGL/OpenGLIndexBuffer.cpp
// =================================================================================
// Реализация индексного буфера OpenGL.
// Инкапсулирует работу с GL_ELEMENT_ARRAY_BUFFER: создание, загрузку данных,
// привязку и удаление. Все вызовы gl* находятся здесь, изолируя остальной
// движок от знания о конкретном API.
//
// Зависимости:
//   - glbinding (OpenGL-функции)
//   - spdlog (логирование ошибок и отладки)
// =================================================================================

// MirEngine/Rendering/OpenGL/OpenGLIndexBuffer.cpp
// =================================================================================
// Реализация OpenGLIndexBuffer.
// =================================================================================

#include "OpenGLIndexBuffer.h"
#include <iostream>

namespace MirEngine {
namespace Rendering {

GLenum OpenGLIndexBuffer::usageToGL(BufferUsage usage)
{
    switch (usage) {
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream:  return GL_STREAM_DRAW;
        case BufferUsage::Static:
        default:                   return GL_STATIC_DRAW;
    }
}

OpenGLIndexBuffer::OpenGLIndexBuffer()
{
    glGenBuffers(1, &m_handle);
    if (m_handle == 0) {
        std::cerr << "[OpenGLIndexBuffer] Failed to generate buffer\n";
    }
}

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
    if (m_handle != 0) {
        glDeleteBuffers(1, &m_handle);
        m_handle = 0;
    }
}

void OpenGLIndexBuffer::uploadIndices(const std::vector<uint32_t>& indices,
                                      BufferUsage usage)
{
    uploadIndices(indices.data(), indices.size(), usage);
}

void OpenGLIndexBuffer::uploadIndices(const uint32_t* data, size_t count,
                                      BufferUsage usage)
{
    if (!data || count == 0) {
        m_indexCount = 0;
        return;
    }

    bind();

    const GLsizeiptr sizeInBytes = static_cast<GLsizeiptr>(count * sizeof(uint32_t));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, data, usageToGL(usage));

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "[OpenGLIndexBuffer] glBufferData error: 0x"
                  << std::hex << err << std::dec << "\n";
        m_indexCount = 0;
    } else {
        m_indexCount = count;
    }

    unbind();
}

void OpenGLIndexBuffer::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle);
}

void OpenGLIndexBuffer::unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

size_t OpenGLIndexBuffer::getSize() const
{
    return m_indexCount * sizeof(uint32_t);
}

} // namespace Rendering
} // namespace MirEngine