
#pragma once

#include "../Resources/IndexBuffer.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

class OpenGLIndexBuffer final : public IndexBuffer {
public:
    OpenGLIndexBuffer();
    ~OpenGLIndexBuffer() override;

    OpenGLIndexBuffer(const OpenGLIndexBuffer&) = delete;
    OpenGLIndexBuffer& operator=(const OpenGLIndexBuffer&) = delete;

    void uploadIndices(const std::vector<uint32_t>& indices,
                       BufferUsage usage = BufferUsage::Static) override;

    void uploadIndices(const uint32_t* data, size_t count,
                       BufferUsage usage = BufferUsage::Static) override;

    void bind()   override;
    void unbind() override;

    void upload(const void* data, size_t size,
                BufferUsage usage = BufferUsage::Static) override;

    [[nodiscard]] size_t getIndexCount() const override { return m_indexCount; }
    [[nodiscard]] size_t getSize()       const override;

    [[nodiscard]] bool isValid() const override {
        return m_handle != 0 && m_indexCount > 0;
    }

    [[nodiscard]] GLuint handle() const noexcept { return m_handle; }

private:
    GLuint m_handle     = 0;
    size_t m_indexCount = 0;

    static GLenum usageToGL(BufferUsage usage);
};

}
}