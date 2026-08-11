// MirEngine/Rendering/OpenGL/OpenGLContext.h
// =================================================================================
// Абстрактный интерфейс OpenGL-контекста.
//
// Скрывает платформенные детали создания и управления OpenGL-контекстом
// (macOS NSOpenGL / CGL, Windows WGL, Linux GLX/EGL и т.д.).
//
// Конкретные реализации:
//   • MacOpenGLContext   (Platform/macOS/OpenGL)
//   • (в будущем) WinOpenGLContext, LinuxOpenGLContext
//
// Используется OpenGLDevice и экспортируется в C API (MirEngineExports).
// =================================================================================

#pragma once

#include <cstdint>

namespace MirEngine {
namespace Rendering {

// ---------------------------------------------------------------------------------
// Размер в пикселях
// ---------------------------------------------------------------------------------
struct Size2D {
    uint32_t width  = 1;
    uint32_t height = 1;

    constexpr Size2D() noexcept = default;
    constexpr Size2D(uint32_t w, uint32_t h) noexcept : width(w), height(h) {}
};

// Нативный дескриптор окна / view (NSView*, HWND, Window и т.д.)
using NativeWindowHandle = void*;

// ---------------------------------------------------------------------------------
// Абстрактный OpenGL-контекст
// ---------------------------------------------------------------------------------
class OpenGLContext {
public:
    virtual ~OpenGLContext() = default;

    // Инициализация контекста для указанного нативного окна/view.
    // Возвращает true при успехе.
    virtual bool initialize(NativeWindowHandle window, const Size2D& size) = 0;

    // Делает контекст текущим в вызывающем потоке.
    virtual void makeCurrent() = 0;

    // Меняет буферы (present).
    virtual void swapBuffers() = 0;

    // Изменяет размер framebuffer / viewport.
    virtual void resize(const Size2D& size) = 0;

    // Текущий размер.
    [[nodiscard]] virtual Size2D size() const = 0;

    // Запрет копирования
    OpenGLContext(const OpenGLContext&) = delete;
    OpenGLContext& operator=(const OpenGLContext&) = delete;

protected:
    OpenGLContext() = default;
};

} // namespace Rendering
} // namespace MirEngine