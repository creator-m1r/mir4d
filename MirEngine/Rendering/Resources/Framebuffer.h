// MirEngine/Rendering/Resources/Framebuffer.h
// =================================================================================
// Абстрактный интерфейс framebuffer (render target).
//
// Позволяет рендерить не в окно, а в текстуру (например, для offscreen pass,
// постобработки, пикинга). Реализации скрывают FBO (OpenGL) / render target (Metal).
//
// Правило изоляции:
//   Никаких GL/MTL/Vk-типов или вызовов в этом заголовке.
// =================================================================================

#pragma once

#include <cstdint>
#include <memory>

namespace MirEngine {
namespace Rendering {

class Texture;

class Framebuffer {
public:
    virtual ~Framebuffer() = default;

    // Создаёт framebuffer с цветовым attachment'ом заданного размера
    // и depth-renderbuffer'ом. При повторном вызове пересоздаёт ресурсы.
    virtual bool create(std::uint32_t width,
                        std::uint32_t height) = 0;

    // Привязывает цветовую текстуру как colour attachment 0.
    virtual void attachColorTexture(const std::shared_ptr<Texture>& colorTexture) = 0;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    virtual void resize(std::uint32_t width, std::uint32_t height) = 0;

    [[nodiscard]] virtual std::uint32_t width() const = 0;
    [[nodiscard]] virtual std::uint32_t height() const = 0;
    [[nodiscard]] virtual std::shared_ptr<Texture> colorTexture() const = 0;
    [[nodiscard]] virtual bool valid() const = 0;

protected:
    Framebuffer() = default;
};

} // namespace Rendering
} // namespace MirEngine