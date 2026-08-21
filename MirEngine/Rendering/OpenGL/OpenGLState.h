
#pragma once

#include <cstdint>

#include "../Core/RenderDevice.h"

namespace MirEngine::Rendering {

class OpenGLState {
public:

    void initialize();

    void setViewport(std::uint32_t width, std::uint32_t height);

    void clear(const ColorRGBA& color,
               float depth,
               int stencil,
               RenderDevice::ClearFlags flags);

    void beginFrame();
    void endFrame();

    void setWireframe(bool enabled);
    void setDepthTest(bool enabled);
    void setBlend(bool enabled);
    void setLineWidth(float width);
    void setCullFace(bool enabled);
    void setDepthFunc(RenderDevice::DepthFunc func);

    [[nodiscard]] std::uint32_t width() const noexcept { return m_width; }
    [[nodiscard]] std::uint32_t height() const noexcept { return m_height; }

private:
    std::uint32_t m_width{0};
    std::uint32_t m_height{0};
    bool m_wireframe{false};
    bool m_depthTest{true};
    bool m_blend{false};
    bool m_cullFace{true};
    RenderDevice::DepthFunc m_depthFunc{RenderDevice::DepthFunc::Less};
    bool m_initialized{false};
};

}