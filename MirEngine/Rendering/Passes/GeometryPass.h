#pragma once

#include "RenderPass.h"
#include "../Core/RenderCommand.h"
#include "../Resources/ShaderLibrary.h"
#include "../Material/MaterialLibrary.hpp"
#include "MirEngine/Core/Identity/ObjectId.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Math/Transform.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace MirEngine::Rendering
{

class RenderDevice;
class RenderContext;
class VertexArray;
class Shader;

/// Canonical scene geometry pass.
///
/// Consumes the canonical mir::Scene and materializes GPU resources keyed by
/// the canonical mir4d::ObjectId. Mesh uploads are cached against the scene
/// content revision, so unchanged scenes are drawn without re-upload.
///
/// Rendering is camera-relative: model translations are shifted by the camera
/// position in double precision on the CPU and the view matrix keeps only its
/// rotation, so huge world coordinates never reach the GPU.
///
/// Selection highlight:
///   - objects present in the RenderContext selection set get a CAD-style
///     selection tint;
///   - when the context carries a source face id (selectionFaceId), only the
///     triangles of that face are tinted instead of the whole object
///     (drawn as an overlay pass with depth func LessEqual);
///   - the object under the cursor (RenderContext hoverObjectId) gets a
///     subtler hover tint unless it is already selected.
class GeometryPass final : public RenderPass
{
public:
    explicit GeometryPass(ShaderLibrary& shaderLibrary);

    bool initialize(RenderDevice& device);
    void execute(RenderContext& context,
                 mir::Scene& scene,
                 RenderDevice& device) override;

    void invalidateSceneCache() noexcept;

    /// Assigns a MaterialLibrary material to an object (default: Engineering Steel).
    void setObjectMaterial(mir4d::ObjectId objectId, MaterialId materialId) noexcept
    {
        m_objectMaterials[objectId] = materialId;
    }

private:
    static constexpr std::uint32_t kDefaultMaterialHandle = 100;
    static constexpr std::uint32_t kHighlightMeshHandle = 0xFFFFFFFE;

    ShaderLibrary& m_shaderLibrary;
    MeshHandle m_nextMeshHandle{1};

    std::unordered_map<mir4d::ObjectId, MeshHandle> m_objectToHandle;
    std::unordered_map<MeshHandle, std::shared_ptr<VertexArray>> m_vaos;
    std::unordered_map<mir4d::ObjectId, MaterialId> m_objectMaterials;
    const mir::Scene* m_cachedScene{nullptr};
    std::uint64_t m_cachedRevision{0};

    // Face-level highlight cache (rebuilt when the selection face changes).
    std::shared_ptr<VertexArray> m_highlightVAO;
    mir4d::ObjectId m_highlightObject{mir4d::InvalidObjectId};
    std::uint64_t m_highlightFace{0};

    void processNode(const mir::ModelNode& node,
                     RenderContext& context,
                     RenderDevice& device,
                     Shader* shader,
                     bool selected,
                     bool hovered);

    void rebuildSceneCache(mir::Scene& scene, RenderDevice& device);
    void uploadMesh(const mir::ModelNode& node,
                    RenderDevice& device);
    void rebuildHighlightFace(mir::Scene& scene, RenderDevice& device);
    void drawHighlightFace(mir::Scene& scene,
                           RenderContext& context,
                           RenderDevice& device,
                           Shader* shader);

    [[nodiscard]] static Matrix4Raw makeModelMatrix(const mir::Transform& transform,
                                                    const double cameraPos[3]) noexcept;

    /// Compatibility overload for RenderContext's float camera position.
    /// Conversion to double happens before subtracting the world-space camera
    /// position, preserving the double-precision camera-relative calculation.
    [[nodiscard]] static Matrix4Raw makeModelMatrix(const mir::Transform& transform,
                                                    const float cameraPos[3]) noexcept
    {
        const double cameraPosDouble[3] = {
            static_cast<double>(cameraPos[0]),
            static_cast<double>(cameraPos[1]),
            static_cast<double>(cameraPos[2])
        };
        return makeModelMatrix(transform, cameraPosDouble);
    }
};

} // namespace MirEngine::Rendering
