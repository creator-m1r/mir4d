
#pragma once

#include <cstdint>

namespace MirEngine {
namespace Rendering {

struct Size2D {
    uint32_t width  = 1;
    uint32_t height = 1;

    constexpr Size2D() noexcept = default;
    constexpr Size2D(uint32_t w, uint32_t h) noexcept : width(w), height(h) {}
};

using NativeWindowHandle = void*;

class OpenGLContext {
public:
    virtual ~OpenGLContext() = default;

    virtual bool initialize(NativeWindowHandle window, const Size2D& size) = 0;

    virtual void makeCurrent() = 0;

    virtual void setView(NativeWindowHandle window) = 0;

    virtual void swapBuffers() = 0;

    virtual void resize(const Size2D& size) = 0;

    [[nodiscard]] virtual Size2D size() const = 0;

    OpenGLContext(const OpenGLContext&) = delete;
    OpenGLContext& operator=(const OpenGLContext&) = delete;

protected:
    OpenGLContext() = default;
};

}
}