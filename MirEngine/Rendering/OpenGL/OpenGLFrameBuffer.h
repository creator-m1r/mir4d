// MirEngine/Rendering/OpenGL/OpenGLFrameBuffer.h
// =================================================================================
// Конкретная реализация Framebuffer для OpenGL (FBO + depth renderbuffer).
// =================================================================================

#pragma once

#include "../Resources/Framebuffer.h"
#include "OpenGLTexture.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include <cstdint>
#include <memory>

namespace MirEngine {
namespace Rendering {

class OpenGLFrameBuffer final : public Framebuffer {
public:
    OpenGLFrameBuffer();
    ~OpenGLFrameBuffer() override;

    OpenGLFrameBuffer(const OpenGLFrameBuffer&) = delete;
    OpenGLFrameBuffer& operator=(const OpenGLFrameBuffer&) = delete;

    bool create(std::uint32_t width, std::uint32_t height) override;

    void attachColorTexture(const std::shared_ptr<Texture>& colorTexture) override;

    void bind() override;
    void unbind() override;

    void resize(std::uint32_t width, std::uint32_t height) override;

    [[nodiscard]] std::uint32_t width() const override { return m_width; }
    [[nodiscard]] std::uint32_t height() const override { return m_height; }
    [[nodiscard]] std::shared_ptr<Texture> colorTexture() const override { return m_colorTexture; }
    [[nodiscard]] bool valid() const override { return m_fbo != 0; }

    [[nodiscard]] GLuint handle() const noexcept { return m_fbo; }

private:
    void releaseResources();
    void attachDepthRenderbuffer();

    GLuint m_fbo{0};
    GLuint m_depthRbo{0};
    std::uint32_t m_width{0};
    std::uint32_t m_height{0};
    std::shared_ptr<Texture> m_colorTexture;
};

} // namespace Rendering
} // namespace MirEngine