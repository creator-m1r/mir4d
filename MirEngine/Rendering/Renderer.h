#pragma once

#include <cstdint>
#include <vector>

namespace mir
{
class Scene;
}

namespace MirEngine::Rendering
{

class RenderContext;

/// Hand-skeleton overlay style (colours / sizes / depth). Defined in
/// RenderContext.h; forward-declared here to keep this header light.
struct HandSkeletonStyle;

/// Backend-independent renderer contract.
/// The engineering scene is always the canonical mir::Scene.
class Renderer
{
public:
    virtual ~Renderer() = default;

    virtual bool initialize() = 0;

    virtual void render(mir::Scene& scene,
                        RenderContext& context) = 0;

    virtual void resize(std::uint32_t width, std::uint32_t height) = 0;

    /// Assigns a library material id to an object. Backends that do not
    /// support material assignment ignore the call.
    virtual void setObjectMaterial(std::uint64_t objectId,
                                   std::int32_t materialId) noexcept
    {
        (void)objectId;
        (void)materialId;
    }

    /// Hand-skeleton overlay style (colours / sizes / depth behaviour).
    /// Backends that do not support the overlay ignore the call.
    virtual void setHandSkeletonStyle(const HandSkeletonStyle& /*style*/) noexcept {}

    /// Hand-skeleton bone topology (single source of truth from Swift).
    virtual void setHandSkeletonTopology(const std::vector<std::pair<int, int>>& /*bones*/) noexcept {}

protected:
    Renderer() = default;
};

} // namespace MirEngine::Rendering
