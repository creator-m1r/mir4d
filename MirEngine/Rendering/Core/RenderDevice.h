
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

class OpenGLContext;

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

class RenderDevice {
public:
    virtual ~RenderDevice() = default;

    virtual bool initialize() = 0;

    enum class ClearFlags : std::uint8_t {
        Color   = 1 << 0,
        Depth   = 1 << 1,
        Stencil = 1 << 2,
        All     = Color | Depth | Stencil
    };

    virtual void clear(const ColorRGBA& color,
                       float depth = 1.0f,
                       int stencil = 0,
                       ClearFlags flags = ClearFlags::All) = 0;

    virtual void draw(const RenderCommand& command) = 0;

    virtual void present() = 0;

    virtual void setViewportSize(std::uint32_t width, std::uint32_t height) = 0;

    virtual void setViewMatrix(const Matrix4Raw& viewMatrix) = 0;
    virtual void setProjectionMatrix(const Matrix4Raw& projMatrix) = 0;

    virtual void setDepthTest(bool enabled) = 0;
    virtual void setBlend(bool enabled) = 0;
    virtual void setWireframe(bool enabled) = 0;
    virtual void setLineWidth(float width) = 0;

    virtual void setCullFace(bool enabled) = 0;

    enum class DepthFunc : std::uint8_t {
        Less = 0,
        LessEqual = 1
    };

    virtual void setDepthFunc(DepthFunc func) = 0;

    virtual std::shared_ptr<VertexBuffer> createVertexBuffer() = 0;
    virtual std::shared_ptr<IndexBuffer> createIndexBuffer() = 0;
    virtual std::shared_ptr<VertexArray> createVertexArray() = 0;

    virtual void registerMesh(MeshHandle handle, std::shared_ptr<VertexArray> mesh) = 0;
    virtual void registerMaterial(MaterialHandle handle, std::shared_ptr<Shader> shader) = 0;

protected:
    RenderDevice() = default;
};

std::unique_ptr<RenderDevice> CreateRenderDevice(OpenGLContext* context);

}

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