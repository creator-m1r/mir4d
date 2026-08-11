// MirEngine/Rendering/OpenGL/OpenGLRenderer.h
// =================================================================================
// Высокоуровневый OpenGL-рендерер.
//
// Владеет OpenGLDevice и управляет циклом кадра.
// Используется из C-API (MirEngineExports) и из SwiftUI/macOS view.
// =================================================================================

#pragma once

#include <memory>
#include <cstdint>

#include "OpenGLContext.h"
#include "OpenGLDevice.h"
#include "../Passes/GridPass.h"          
#include "../Core/RenderContext.h"


namespace MirEngine {
namespace Rendering {

class OpenGLRenderer {
public:
    // Принимает уже созданный контекст (не владеет им).
    explicit OpenGLRenderer(OpenGLContext* context);

    ~OpenGLRenderer();

    // Запрет копирования
    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;

    // Инициализация устройства и начальных состояний.
    // Возвращает true при успехе.
    bool initialize();

    // Выполняет один кадр (clear + проходы + present).
    void render(const RenderContext& ctx);

    // Изменение размера viewport / framebuffer.
    void resize(uint32_t width, uint32_t height);

    // Доступ к устройству (для проходов и отладки).
    [[nodiscard]] OpenGLDevice* device() noexcept { return m_device.get(); }
    [[nodiscard]] const OpenGLDevice* device() const noexcept { return m_device.get(); }

    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    OpenGLContext*               m_context     = nullptr;
    std::unique_ptr<OpenGLDevice> m_device;
    std::unique_ptr<GridPass>    m_gridPass;   // <-- добавлено
    bool                         m_initialized = false;
};

} // namespace Rendering
} // namespace MirEngine