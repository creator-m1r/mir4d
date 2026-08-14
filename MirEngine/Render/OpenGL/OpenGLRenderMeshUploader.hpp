#pragma once

#include "../RenderMeshGPU.hpp"

namespace mir
{

/// OpenGL backend for RenderMeshGPUUploader.
/// The OpenGL context must already be current on the calling thread.
class OpenGLRenderMeshUploader final : public RenderMeshGPUUploader
{
public:
    [[nodiscard]] RenderMeshGPU upload(const RenderMesh& mesh) override;
    void release(RenderMeshGPU& mesh) noexcept override;
};

} // namespace mir
