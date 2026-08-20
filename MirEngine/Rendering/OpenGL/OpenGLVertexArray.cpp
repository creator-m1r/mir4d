// MirEngine/Rendering/OpenGL/OpenGLVertexArray.cpp
// =================================================================================
// Реализация вершинного массива OpenGL (VAO).
//
// Связывает вершинный и индексный буферы, настраивает расположение атрибутов
// с использованием современного подхода OpenGL 4.1:
//   - glVertexAttribFormat (формат атрибута)
//   - glVertexAttribBinding (привязка атрибута к binding point)
//   - glBindVertexBuffer (привязка буфера к binding point)
//   - glEnableVertexAttribArray
//
// Это заменяет устаревший glVertexAttribPointer и даёт более гибкий контроль.
//
// Зависимости:
//   - glbinding (OpenGL-функции)
//   - spdlog (логирование)
// =================================================================================


#include "OpenGLVertexArray.h"
#include "OpenGLVertexBuffer.h"
#include "OpenGLIndexBuffer.h"
#include "../Resources/Vertex.h"

#include <iostream>

namespace MirEngine {
namespace Rendering {

namespace
{
GLuint g_defaultVAO = 0;
void bindDefaultVertexArrayImpl()
{
    if (g_defaultVAO == 0)
        glGenVertexArrays(1, &g_defaultVAO);
    glBindVertexArray(g_defaultVAO);
}
} // namespace

void BindDefaultVertexArray() noexcept
{
    bindDefaultVertexArrayImpl();
}

void ResetDefaultVertexArray() noexcept
{
    // Drop the handle without a GL call: the VAO is owned by the (now destroyed)
    // context and is freed with it. The next BindDefaultVertexArray() will
    // lazily regenerate a fresh VAO in the new context.
    g_defaultVAO = 0;
}

// --------------------------------------------------------------------------
// Конструктор / деструктор
// --------------------------------------------------------------------------
OpenGLVertexArray::OpenGLVertexArray()
{
    glGenVertexArrays(1, &m_vao);
    if (m_vao == 0) {
        std::cerr << "[OpenGLVertexArray] Failed to create VAO\n";
    }
}

OpenGLVertexArray::~OpenGLVertexArray()
{
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

// --------------------------------------------------------------------------
// bind / unbind
// --------------------------------------------------------------------------
void OpenGLVertexArray::bind()
{
    glBindVertexArray(m_vao);
}

void OpenGLVertexArray::unbind()
{
    // Bind the persistent scratch VAO instead of 0: OpenGL 4.1 Core has no
    // default VAO, and glBindBuffer/glBufferData require a bound VAO.
    bindDefaultVertexArrayImpl();
}

// --------------------------------------------------------------------------
// Установка вершинного буфера + настройка атрибутов
// --------------------------------------------------------------------------
void OpenGLVertexArray::setVertexBuffer(std::shared_ptr<VertexBuffer> vb)
{
    m_vertexBuffer = std::move(vb);
    if (!m_vertexBuffer) {
        return;
    }

    auto* glVB = dynamic_cast<OpenGLVertexBuffer*>(m_vertexBuffer.get());
    if (!glVB) {
        std::cerr << "[OpenGLVertexArray] VertexBuffer is not OpenGLVertexBuffer\n";
        m_vertexBuffer.reset();
        return;
    }

    bind();
    setupAttributes();
    unbind();
}

// --------------------------------------------------------------------------
// Установка индексного буфера
// --------------------------------------------------------------------------
void OpenGLVertexArray::setIndexBuffer(std::shared_ptr<IndexBuffer> ib)
{
    m_indexBuffer = std::move(ib);
    if (!m_indexBuffer) {
        return;
    }

    auto* glIB = dynamic_cast<OpenGLIndexBuffer*>(m_indexBuffer.get());
    if (!glIB) {
        std::cerr << "[OpenGLVertexArray] IndexBuffer is not OpenGLIndexBuffer\n";
        m_indexBuffer.reset();
        return;
    }

    // Привязываем индексный буфер к VAO
    bind();
    glIB->bind();          // GL_ELEMENT_ARRAY_BUFFER
    unbind();
    // После unbind VAO индексный буфер остаётся привязанным к этому VAO
}

// --------------------------------------------------------------------------
// Количество элементов для draw call
// --------------------------------------------------------------------------
uint32_t OpenGLVertexArray::getElementCount() const
{
    if (m_indexBuffer) {
        return static_cast<uint32_t>(m_indexBuffer->getIndexCount());
    }
    if (m_vertexBuffer) {
        return static_cast<uint32_t>(m_vertexBuffer->getVertexCount());
    }
    return 0;
}

// --------------------------------------------------------------------------
// Настройка атрибутов под структуру Vertex
// layout:
//   location 0 : position (vec3)  offset 0
//   location 1 : normal   (vec3)  offset 12
//   location 2 : uv       (vec2)  offset 24
// --------------------------------------------------------------------------
void OpenGLVertexArray::setupAttributes()
{
    auto* glVB = dynamic_cast<OpenGLVertexBuffer*>(m_vertexBuffer.get());
    if (!glVB) return;

    bind();
    glVB->bind();

    // --- position (location = 0) ---
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(sizeof(Vertex)),
                          reinterpret_cast<const void*>(offsetof(Vertex, position)));

    // --- normal (location = 1) ---
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(sizeof(Vertex)),
                          reinterpret_cast<const void*>(offsetof(Vertex, normal)));

    // --- uv (location = 2) ---
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(sizeof(Vertex)),
                          reinterpret_cast<const void*>(offsetof(Vertex, uv)));

    glVB->unbind();
    unbind();
}

} // namespace Rendering
} // namespace MirEngine