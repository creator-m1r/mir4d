#pragma once

#include "RenderMesh.hpp"

#include <cstdint>

namespace MirEngine {
namespace Rendering {

struct GPUBufferHandle
{
    std::uint64_t value{0};

    [[nodiscard]] bool valid() const noexcept
    {
        return value != 0;
    }
};

struct RenderMeshGPU
{
    GPUBufferHandle vertexArray{};
    GPUBufferHandle vertexBuffer{};
    GPUBufferHandle indexBuffer{};
    std::uint32_t indexCount{0};

    [[nodiscard]] bool valid() const noexcept
    {
        return vertexArray.valid() &&
               vertexBuffer.valid() &&
               indexBuffer.valid() &&
               indexCount > 0;
    }
};

/// Rendering backend-neutral GPU resource interface.
/// Concrete OpenGL/Metal/Vulkan implementations live outside this layer.
class RenderMeshGPUUploader
{
public:
    virtual ~RenderMeshGPUUploader() = default;

    [[nodiscard]] virtual RenderMeshGPU upload(
        const RenderMesh& mesh) = 0;

    virtual void release(RenderMeshGPU& mesh) noexcept = 0;
};

} // namespace Rendering
} // namespace MirEngine
