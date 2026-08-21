
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
}

void BindDefaultVertexArray() noexcept
{
    bindDefaultVertexArrayImpl();
}

void ResetDefaultVertexArray() noexcept
{

    g_defaultVAO = 0;
}

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

void OpenGLVertexArray::bind()
{
    glBindVertexArray(m_vao);
}

void OpenGLVertexArray::unbind()
{

    bindDefaultVertexArrayImpl();
}

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

    bind();
    glIB->bind();
    unbind();

}

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

void OpenGLVertexArray::setupAttributes()
{
    auto* glVB = dynamic_cast<OpenGLVertexBuffer*>(m_vertexBuffer.get());
    if (!glVB) return;

    bind();
    glVB->bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(sizeof(Vertex)),
                          reinterpret_cast<const void*>(offsetof(Vertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(sizeof(Vertex)),
                          reinterpret_cast<const void*>(offsetof(Vertex, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(sizeof(Vertex)),
                          reinterpret_cast<const void*>(offsetof(Vertex, uv)));

    glVB->unbind();
    unbind();
}

}
}