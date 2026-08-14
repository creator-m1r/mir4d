// MirEngine/Rendering/Core/RenderDevice.h
// =================================================================================
// Абстрактный интерфейс графического устройства (GPU).
//
// Главная точка входа для рендеринга в MirEngine.
// Все команды взаимодействия с GPU проходят через этот интерфейс.
// Конкретные реализации (OpenGLDevice, MetalDevice, VulkanDevice)
// скрывают вызовы API и управление состояниями.
//
// Архитектура:
//   Renderer
//       │
//       ▼
//   RenderDevice  ← этот интерфейс
//       │
//       ├── OpenGLDevice
//       ├── MetalDevice   (будущее)
//       └── VulkanDevice  (будущее)
//
// Обязанности:
//   • Инициализация
//   • Очистка буферов кадра
//   • Выполнение команд рисования (RenderCommand)
//   • Управление viewport и матрицами
//   • Present (swap buffers)
// =================================================================================

#pragma once

#include <cstdint>
#include <memory>

#include "RenderCommand.h"
#include "RenderContext.h"

namespace MirEngine {
namespace Rendering {

// Предварительное объявление (реализация в OpenGL/OpenGLContext.h)
class OpenGLContext;

// ---------------------------------------------------------------------------------
// Простая структура цвета RGBA (временная замена полноценного Color из Math).
// Компоненты в диапазоне [0.0f … 1.0f].
// ---------------------------------------------------------------------------------
struct ColorRGBA {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    constexpr ColorRGBA() noexcept = default;

    constexpr ColorRGBA(float red, float green, float blue, float alpha = 1.0f) noexcept
        : r(red), g(green), b(blue), a(alpha)
    {}
};

// ---------------------------------------------------------------------------------
// Абстрактный класс RenderDevice
// ---------------------------------------------------------------------------------
class RenderDevice {
public:
    virtual ~RenderDevice() = default;

    // --------------------------------------------------------------------------
    // Инициализация устройства.
    // Вызывается один раз после создания контекста и перед первым кадром.
    // Возвращает true при успехе.
    // --------------------------------------------------------------------------
    virtual bool initialize() = 0;

    // --------------------------------------------------------------------------
    // Флаги очистки буферов.
    // --------------------------------------------------------------------------
    enum class ClearFlags : uint8_t {
        Color   = 1 << 0,
        Depth   = 1 << 1,
        Stencil = 1 << 2,
        All     = Color | Depth | Stencil
    };

    // --------------------------------------------------------------------------
    // Очищает текущий буфер кадра.
    // --------------------------------------------------------------------------
    virtual void clear(
        const ColorRGBA& color,
        float depth     = 1.0f,
        int   stencil   = 0,
        ClearFlags flags = ClearFlags::All
    ) = 0;

    // --------------------------------------------------------------------------
    // Выполняет одну команду рендеринга.
    // --------------------------------------------------------------------------
    virtual void draw(const RenderCommand& command) = 0;

    // --------------------------------------------------------------------------
    // Завершает кадр и отображает результат (swapBuffers).
    // --------------------------------------------------------------------------
    virtual void present() = 0;

    // --------------------------------------------------------------------------
    // Устанавливает размер области вывода (viewport).
    // --------------------------------------------------------------------------
    virtual void setViewportSize(uint32_t width, uint32_t height) = 0;

    // --------------------------------------------------------------------------
    // Устанавливает матрицы вида и проекции.
    // --------------------------------------------------------------------------
    virtual void setViewMatrix(const Matrix4Raw& viewMatrix) = 0;
    virtual void setProjectionMatrix(const Matrix4Raw& projMatrix) = 0;

protected:
    RenderDevice() = default;
};

// ---------------------------------------------------------------------------------
// Фабрика создания устройства.
// В текущей реализации возвращает OpenGLDevice.
// ---------------------------------------------------------------------------------
std::unique_ptr<RenderDevice> CreateRenderDevice(OpenGLContext* context);

} // namespace Rendering
} // namespace MirEngine

// Удобные операторы для ClearFlags
inline constexpr MirEngine::Rendering::RenderDevice::ClearFlags
operator|(MirEngine::Rendering::RenderDevice::ClearFlags a,
          MirEngine::Rendering::RenderDevice::ClearFlags b) noexcept
{
    using T = std::underlying_type_t<MirEngine::Rendering::RenderDevice::ClearFlags>;
    return static_cast<MirEngine::Rendering::RenderDevice::ClearFlags>(
        static_cast<T>(a) | static_cast<T>(b));
}

inline constexpr MirEngine::Rendering::RenderDevice::ClearFlags
operator&(MirEngine::Rendering::RenderDevice::ClearFlags a,
          MirEngine::Rendering::RenderDevice::ClearFlags b) noexcept
{
    using T = std::underlying_type_t<MirEngine::Rendering::RenderDevice::ClearFlags>;
    return static_cast<MirEngine::Rendering::RenderDevice::ClearFlags>(
        static_cast<T>(a) & static_cast<T>(b));
}