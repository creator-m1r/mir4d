#pragma once

#include "../Core/Identity/ObjectId.hpp"
#include "../Geometry/Scene/Scene.hpp"
#include "../Math/Point.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mir::rendering
{

/// Document-to-render metadata bridge.
///
/// This component deliberately does not create a second Scene, Mesh or GPU
/// representation. Rendering consumes mir::Scene directly. The bridge only
/// caches document-derived metadata needed by presentation services such as
/// camera framing, status overlays and render scheduling.
class DocumentSceneRenderBridge
{
public:
    struct Bounds
    {
        Point3 min{Point3::origin()};
        Point3 max{Point3::origin()};
        bool valid{false};

        [[nodiscard]] Point3 center() const noexcept
        {
            return {(min.x + max.x) * 0.5,
                    (min.y + max.y) * 0.5,
                    (min.z + max.z) * 0.5};
        }

        [[nodiscard]] double radius() const noexcept;
    };

    DocumentSceneRenderBridge() = default;
    DocumentSceneRenderBridge(const DocumentSceneRenderBridge&) = delete;
    DocumentSceneRenderBridge& operator=(const DocumentSceneRenderBridge&) = delete;

    [[nodiscard]] Bounds rebuild(const Scene& documentScene);

    [[nodiscard]] std::uint64_t sourceRevision() const noexcept { return sourceRevision_; }
    [[nodiscard]] bool hasCachedScene() const noexcept { return sourceRevision_ != InvalidRevision; }
    [[nodiscard]] const std::vector<mir4d::ObjectId>& objectIds() const noexcept { return objectIds_; }

private:
    static constexpr std::uint64_t InvalidRevision = 0;

    std::uint64_t sourceRevision_{InvalidRevision};
    Bounds cachedBounds_{};
    std::vector<mir4d::ObjectId> objectIds_;
};

} // namespace mir::rendering
