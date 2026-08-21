// MirEngine/Rendering/OpenGL/OpenGLState.h
// =================================================================================
// OpenGL global state management.
//
// Encapsulates glEnable / glDisable / glViewport / glClear and the pipeline
// state overrides used by the renderer (depth test, blending, wireframe,
// line width). OpenGLDevice routes its backend-neutral state calls here so
// GL calls never leak into the passes.
// =================================================================================

#pragma once

#include <cstdint>

#include "../Core/RenderDevice.h"   // ColorRGBA, ClearFlags

namespace MirEngine::Rendering {

class OpenGLState {
public:
    // Initializes the default state (depth test, culling, ...).
    void initialize();

    // Sets the viewport.
    void setViewport(std::uint32_t width, std::uint32_t height);

    // Clears buffers with the given color, depth and flags.
    void clear(const ColorRGBA& color,
               float depth,
               int stencil,
               RenderDevice::ClearFlags flags);

    // Frame boundaries (state validation hooks).
    void beginFrame();
    void endFrame();

    // Pipeline state overrides.
    void setWireframe(bool enabled);
    void setDepthTest(bool enabled);
    void setBlend(bool enabled);
    void setLineWidth(float width);
    void setCullFace(bool enabled);
    void setDepthFunc(RenderDevice::DepthFunc func);

    // Current viewport dimensions.
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

} // namespace MirEngine::Rendering