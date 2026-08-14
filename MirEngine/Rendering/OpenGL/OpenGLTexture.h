// MirEngine/Rendering/OpenGL/OpenGLTexture.h
// =================================================================================
// Конкретная реализация Texture для OpenGL.
// =================================================================================

#pragma once

#include "../Resources/Texture.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include <cstdint>

namespace MirEngine {
namespace Rendering {

class OpenGLTexture final : public Texture {
public:
    OpenGLTexture();
    ~OpenGLTexture() override;

    OpenGLTexture(const OpenGLTexture&) = delete;
    OpenGLTexture& operator=(const OpenGLTexture&) = delete;

    bool create(std::uint32_t width,
                std::uint32_t height,
                TextureFormat format = TextureFormat::RGBA8,
                TextureFilter filter = TextureFilter::Linear) override;

    bool upload(const std::uint8_t* pixels) override;

    void bind(std::uint32_t slot = 0) override;
    void unbind() override;

    void resize(std::uint32_t width, std::uint32_t height) override;

    [[nodiscard]] std::uint32_t width() const override { return m_width; }
    [[nodiscard]] std::uint32_t height() const override { return m_height; }
    [[nodiscard]] TextureFormat format() const override { return m_format; }
    [[nodiscard]] bool valid() const override { return m_id != 0; }

    [[nodiscard]] GLuint handle() const noexcept { return m_id; }

private:
    GLuint m_id{0};
    std::uint32_t m_width{0};
    std::uint32_t m_height{0};
    TextureFormat m_format{TextureFormat::RGBA8};
    TextureFilter m_filter{TextureFilter::Linear};
};

} // namespace Rendering
} // namespace MirEngine