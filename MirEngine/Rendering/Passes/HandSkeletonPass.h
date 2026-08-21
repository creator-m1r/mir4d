#pragma once

#include "RenderPass.h"

#include <array>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "../Core/RenderContext.h"

namespace MirEngine::Rendering
{

class Shader;
class RenderDevice;
class VertexBuffer;
class VertexArray;

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

    void setTopology(const std::vector<std::pair<int, int>>& bones);

    void setStyle(const HandSkeletonStyle& style) noexcept { m_style = style; }

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

    static void gestureAccent(int gestureCode, float out[3]) noexcept;
};

}
