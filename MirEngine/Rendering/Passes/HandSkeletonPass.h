#pragma once

#include "RenderPass.h"

#include <array>
#include <cstdint>
#include <memory>

namespace MirEngine::Rendering
{

class Shader;
class RenderDevice;
class VertexBuffer;
class VertexArray;

/// Real-time 3D hand-skeleton overlay (debug / assist mode).
///
/// Consumes `RenderContext::handSkeleton` — a transient sensor view pushed from
/// the hand-tracking subsystem. It draws in true scene space using the frame's
/// view-projection matrix, so the skeleton aligns with the CAD model. It never
/// touches the scene, Document or command history.
///
/// Drawn as GL_POINTS (joints) and GL_LINES (bones + optional pointing ray).
/// Bones use a static topology indexed by LandmarkID.allCases order. When the
/// mode is Off or no hands are tracked the pass is a zero-cost no-op.
class HandSkeletonPass final : public RenderPass
{
public:
    explicit HandSkeletonPass() = default;
    ~HandSkeletonPass() override;

    bool initialize(RenderDevice& device);

    void execute(RenderContext& context,
                 mir::Scene& scene,
                 RenderDevice& device) override;

    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

    /// Sets the bone topology. `bones` is a list of (parent, child) index pairs
    /// referencing the 21-joint array ordered by LandmarkID.allCases (see
    /// MIRHandSkeletonTopology on the Swift side). Replaces the default topology.
    void setTopology(const std::vector<std::pair<int, int>>& bones);

    /// Sets the visual style (colours, sizes, transparency, depth behaviour).
    void setStyle(const HandSkeletonStyle& style) noexcept { m_style = style; }

    /// Single-source bone topology: defaults to the canonical 23-bone hand and
    /// is overridden from Swift (MIRHandSkeletonTopology) at runtime.
    const std::vector<std::pair<int, int>>& boneIndices() const noexcept { return m_boneIndices; }

private:
    bool m_initialized{false};

    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<VertexBuffer> m_pointVBO;
    std::shared_ptr<VertexArray> m_pointVAO;
    std::shared_ptr<VertexBuffer> m_lineVBO;
    std::shared_ptr<VertexArray> m_lineVAO;

    HandSkeletonStyle m_style{};
    std::vector<std::pair<int, int>> m_boneIndices = defaultBoneIndices();

    static constexpr float kMinConfidence = 0.4f;

    static std::vector<std::pair<int, int>> defaultBoneIndices();

    // Accent colour for the active gesture (used for the pinch line / tip tint).
    static void gestureAccent(int gestureCode, float out[3]) noexcept;
};

} // namespace MirEngine::Rendering
