#pragma once

#include "../Resources/RenderMeshGPU.hpp"

namespace MirEngine {
namespace Rendering {

/// OpenGL backend for RenderMeshGPUUploader.
/// The OpenGL context must already be current on the calling thread.
class OpenGLRenderMeshUploader final : public RenderMeshGPUUploader
{
public:
    [[nodiscard]] RenderMeshGPU upload(const RenderMesh& mesh) override;
    void release(RenderMeshGPU& mesh) noexcept override;
};

} // namespace Rendering
} // namespace MirEngine
