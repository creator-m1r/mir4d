
#pragma once

#include "../Resources/VertexBuffer.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

class OpenGLVertexBuffer final : public VertexBuffer {
public:
    OpenGLVertexBuffer();
    ~OpenGLVertexBuffer() override;

    OpenGLVertexBuffer(const OpenGLVertexBuffer&) = delete;
    OpenGLVertexBuffer& operator=(const OpenGLVertexBuffer&) = delete;

    void uploadVertices(const std::vector<Vertex>& vertices,
                        BufferUsage usage = BufferUsage::Static) override;

    void uploadVertices(const Vertex* data, size_t count,
                        BufferUsage usage = BufferUsage::Static) override;

    void bind()   override;
    void unbind() override;

    void upload(const void* data, size_t size,
                BufferUsage usage = BufferUsage::Static) override;

    [[nodiscard]] size_t getVertexCount() const override { return m_vertexCount; }
    [[nodiscard]] size_t getVertexSize()  const override { return sizeof(Vertex); }
    [[nodiscard]] size_t getSize()        const override;

    [[nodiscard]] bool isValid() const override {
        return m_handle != 0 && m_vertexCount > 0;
    }

    [[nodiscard]] GLuint handle() const noexcept { return m_handle; }

private:
    GLuint m_handle      = 0;
    size_t m_vertexCount = 0;

    static GLenum usageToGL(BufferUsage usage);
};

}
}