#pragma once

#include "RenderPass.h"
#include "../Core/RenderCommand.h"
#include "../Resources/ShaderLibrary.h"
#include "MirEngine/Core/Identity/ObjectId.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Math/Transform.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace MirEngine::Rendering
{

class RenderDevice;
class RenderContext;
class VertexArray;
class Shader;

/// Canonical scene geometry pass.
/// Consumes the canonical mir::Scene/model aliases and materializes GPU
/// resources by the canonical mir4d::ObjectId.
class GeometryPass final : public RenderPass
{
public:
    explicit GeometryPass(ShaderLibrary& shaderLibrary);

    bool initialize(RenderDevice& device);
    void execute(RenderContext& context,
                 mir::Scene& scene,
                 RenderDevice& device) override;

    void invalidateSceneCache() noexcept;

private:
    ShaderLibrary& m_shaderLibrary;
    MaterialHandle m_defaultMaterial{0};
    MeshHandle m_nextMeshHandle{1};

    std::unordered_map<mir4d::ObjectId, MeshHandle> m_objectToHandle;
    std::unordered_map<MeshHandle, std::shared_ptr<VertexArray>> m_vaos;
    const mir::Scene* m_cachedScene{nullptr};
    std::uint64_t m_cachedRevision{0};

    void processNode(const mir::ModelNode& node,
                     RenderDevice& device,
                     Shader* shader);

    [[nodiscard]] static Matrix4Raw makeModelMatrix(const mir::Transform& transform) noexcept;
};

} // namespace MirEngine::Rendering
