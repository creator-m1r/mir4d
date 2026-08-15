// MirEngine/Rendering/Core/RenderDevice.h
// =================================================================================
// Abstract graphics device (GPU) interface.
//
// Main entry point for rendering inside MirEngine. All GPU interaction goes
// through this interface; concrete implementations (OpenGLDevice, and in the
// future MetalDevice / VulkanDevice) hide the underlying API and state
// management.
//
// Architecture:
//   Renderer
//       |
//       v
//   RenderDevice   <-- this interface
//       |
//       +-- OpenGLDevice
//       +-- MetalDevice   (future)
//       +-- VulkanDevice  (future)
//
// Responsibilities:
//   - initialization;
//   - frame buffer clearing;
//   - executing draw commands (RenderCommand);
//   - viewport and matrix management;
//   - pipeline state (depth test, blending, wireframe, line width);
//   - presenting (swap buffers).
// =================================================================================

#pragma once

#include <cstdint>
#include <memory>

#include "RenderCommand.h"
#include "RenderContext.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/IndexBuffer.h"
#include "../Resources/VertexArray.h"
#include "../Resources/Shader.h"

namespace MirEngine::Rendering {

// Forward declaration (implementation in OpenGL/OpenGLContext.h)
class OpenGLContext;

// -----------------------------------------------------------------------------
// Simple RGBA color (temporary stand-in for the canonical Color type).
// Components are in [0.0f ... 1.0f].
// -----------------------------------------------------------------------------
struct ColorRGBA {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
    float a{1.0f};

    constexpr ColorRGBA() noexcept = default;

    constexpr ColorRGBA(float red, float green, float blue, float alpha = 1.0f) noexcept
        : r(red), g(green), b(blue), a(alpha)
    {
    }
};

// -----------------------------------------------------------------------------
// Abstract graphics device.
// -----------------------------------------------------------------------------
class RenderDevice {
public:
    virtual ~RenderDevice() = default;

    // --------------------------------------------------------------------------
    // Initializes the device. Called once after context creation and before
    // the first frame. Returns true on success.
    // --------------------------------------------------------------------------
    virtual bool initialize() = 0;

    // --------------------------------------------------------------------------
    // Frame buffer clear flags.
    // --------------------------------------------------------------------------
    enum class ClearFlags : std::uint8_t {
        Color   = 1 << 0,
        Depth   = 1 << 1,
        Stencil = 1 << 2,
        All     = Color | Depth | Stencil
    };

    // --------------------------------------------------------------------------
    // Clears the current frame buffer.
    // --------------------------------------------------------------------------
    virtual void clear(const ColorRGBA& color,
                       float depth = 1.0f,
                       int stencil = 0,
                       ClearFlags flags = ClearFlags::All) = 0;

    // --------------------------------------------------------------------------
    // Executes one draw command.
    // --------------------------------------------------------------------------
    virtual void draw(const RenderCommand& command) = 0;

    // --------------------------------------------------------------------------
    // Finishes the frame and presents the result (swapBuffers).
    // --------------------------------------------------------------------------
    virtual void present() = 0;

    // --------------------------------------------------------------------------
    // Sets the viewport size.
    // --------------------------------------------------------------------------
    virtual void setViewportSize(std::uint32_t width, std::uint32_t height) = 0;

    // --------------------------------------------------------------------------
    // Sets the view / projection matrices.
    // --------------------------------------------------------------------------
    virtual void setViewMatrix(const Matrix4Raw& viewMatrix) = 0;
    virtual void setProjectionMatrix(const Matrix4Raw& projMatrix) = 0;

    // --------------------------------------------------------------------------
    // Pipeline state control (backend-neutral).
    // --------------------------------------------------------------------------
    virtual void setDepthTest(bool enabled) = 0;
    virtual void setBlend(bool enabled) = 0;
    virtual void setWireframe(bool enabled) = 0;
    virtual void setLineWidth(float width) = 0;

    // Depth comparison function (coverage / overlay rendering).
    enum class DepthFunc : std::uint8_t {
        Less = 0,       // default: front-most surfaces win
        LessEqual = 1   // same-depth overlays (e.g. selected face highlight)
    };

    virtual void setDepthFunc(DepthFunc func) = 0;

    // --------------------------------------------------------------------------
    // GPU resource factories.
    // --------------------------------------------------------------------------
    virtual std::shared_ptr<VertexBuffer> createVertexBuffer() = 0;
    virtual std::shared_ptr<IndexBuffer> createIndexBuffer() = 0;
    virtual std::shared_ptr<VertexArray> createVertexArray() = 0;

    // --------------------------------------------------------------------------
    // Registers meshes and materials in the device cache.
    // --------------------------------------------------------------------------
    virtual void registerMesh(MeshHandle handle, std::shared_ptr<VertexArray> mesh) = 0;
    virtual void registerMaterial(MaterialHandle handle, std::shared_ptr<Shader> shader) = 0;

protected:
    RenderDevice() = default;
};

// -----------------------------------------------------------------------------
// Device factory. The current implementation returns an OpenGLDevice.
// -----------------------------------------------------------------------------
std::unique_ptr<RenderDevice> CreateRenderDevice(OpenGLContext* context);

} // namespace MirEngine::Rendering

// Convenience operators for ClearFlags
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