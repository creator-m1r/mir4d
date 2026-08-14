// MirEngine/Rendering/Resources/Texture.h
// =================================================================================
// Абстрактный интерфейс текстуры 2D.
//
// Инкапсулирует создание GPU-текстуры, загрузку пиксельных данных и привязку
// к текстурному слоту. Конкретные реализации (OpenGLTexture, MetalTexture)
// скрывают вызовы соответствующего графического API.
//
// Правило изоляции:
//   Никаких GL/MTL/Vk-типов или вызовов в этом заголовке.
// =================================================================================

#pragma once

#include <cstdint>

namespace MirEngine {
namespace Rendering {

enum class TextureFormat {
    RGBA8,   // 32-битный цвет с альфа-каналом
    RGB8,    // 24-битный цвет
    Depth24  // 24-битный depth (для render targets)
};

enum class TextureFilter {
    Nearest,
    Linear
};

class Texture {
public:
    virtual ~Texture() = default;

    // Создаёт GPU-текстуру заданного размера и формата.
    virtual bool create(std::uint32_t width,
                        std::uint32_t height,
                        TextureFormat format = TextureFormat::RGBA8,
                        TextureFilter filter = TextureFilter::Linear) = 0;

    // Загружает пиксельные данные (RGBA8/RGB8: по width*height*components байт;
    // Depth24: данные не загружаются, текстура используется как depth attachment).
    virtual bool upload(const std::uint8_t* pixels) = 0;

    // Привязывает текстуру к текстурному слоту (0..15).
    virtual void bind(std::uint32_t slot = 0) = 0;
    virtual void unbind() = 0;

    virtual void resize(std::uint32_t width, std::uint32_t height) = 0;

    [[nodiscard]] virtual std::uint32_t width() const = 0;
    [[nodiscard]] virtual std::uint32_t height() const = 0;
    [[nodiscard]] virtual TextureFormat format() const = 0;
    [[nodiscard]] virtual bool valid() const = 0;

protected:
    Texture() = default;
};

} // namespace Rendering
} // namespace MirEngine