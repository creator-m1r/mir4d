// MirEngine/Rendering/OpenGL/OpenGLState.h
// =================================================================================
// Управление глобальным состоянием OpenGL.
//
// Инкапсулирует вызовы glEnable / glDisable / glViewport / glClear и т.д.
// OpenGLDevice использует этот класс, чтобы не размазывать GL-вызовы по коду.
// =================================================================================

#pragma once

#include <cstdint>
#include "../Core/RenderDevice.h"   // ColorRGBA, ClearFlags

namespace MirEngine {
namespace Rendering {

class OpenGLState {
public:
    // Инициализация начальных состояний (depth test, cull face и т.д.)
    void initialize();

    // Установка viewport
    void setViewport(uint32_t width, uint32_t height);

    // Очистка буферов с заданным цветом и флагами
    void clear(const ColorRGBA& color,
               float depth,
               int stencil,
               RenderDevice::ClearFlags flags);

    // Начало кадра (можно использовать для сброса состояний)
    void beginFrame();

    // Конец кадра
    void endFrame();

    // Включение / выключение проволочного режима
    void setWireframe(bool enabled);

    // Текущие размеры viewport
    [[nodiscard]] uint32_t width()  const noexcept { return m_width; }
    [[nodiscard]] uint32_t height() const noexcept { return m_height; }

private:
    uint32_t m_width  = 0;
    uint32_t m_height = 0;
    bool     m_wireframe = false;
};

} // namespace Rendering
} // namespace MirEngine