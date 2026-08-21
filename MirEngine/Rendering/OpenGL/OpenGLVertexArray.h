
#pragma once

#include "../Resources/VertexArray.h"
#include <memory>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

class OpenGLVertexArray final : public VertexArray {
public:
    OpenGLVertexArray();
    ~OpenGLVertexArray() override;

    OpenGLVertexArray(const OpenGLVertexArray&) = delete;
    OpenGLVertexArray& operator=(const OpenGLVertexArray&) = delete;

    void bind()   override;
    void unbind() override;

    void setVertexBuffer(std::shared_ptr<VertexBuffer> vb) override;
    void setIndexBuffer (std::shared_ptr<IndexBuffer>  ib) override;

    [[nodiscard]] std::shared_ptr<VertexBuffer> getVertexBuffer() const override {
        return m_vertexBuffer;
    }
    [[nodiscard]] std::shared_ptr<IndexBuffer> getIndexBuffer() const override {
        return m_indexBuffer;
    }

    [[nodiscard]] uint32_t getElementCount() const override;
    [[nodiscard]] bool     hasIndexBuffer()  const override {
        return m_indexBuffer != nullptr;
    }

    [[nodiscard]] bool isValid() const override {
        return m_vao != 0 && m_vertexBuffer != nullptr;
    }

    [[nodiscard]] GLuint handle() const noexcept { return m_vao; }

private:
    GLuint m_vao = 0;
    std::shared_ptr<VertexBuffer> m_vertexBuffer;
    std::shared_ptr<IndexBuffer>  m_indexBuffer;

    void setupAttributes();
};

void BindDefaultVertexArray() noexcept;

void ResetDefaultVertexArray() noexcept;

}
}