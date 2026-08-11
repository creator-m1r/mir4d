#pragma once

#include "../../../Rendering/OpenGL/OpenGLContext.h"

namespace MirEngine {
namespace Platform {
namespace macOS {

class MacOpenGLContext final : public Rendering::OpenGLContext {
public:
    MacOpenGLContext();
    ~MacOpenGLContext() override;

    bool initialize(Rendering::NativeWindowHandle window,
                    const Rendering::Size2D& size) override;

    void makeCurrent() override;
    void swapBuffers() override;
    void resize(const Rendering::Size2D& size) override;

    [[nodiscard]] Rendering::Size2D size() const override;

private:
    struct Impl;
    Impl* m_impl = nullptr;
};

} // namespace macOS
} // namespace Platform
} // namespace MirEngine