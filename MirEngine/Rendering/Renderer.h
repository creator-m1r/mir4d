#pragma once

#include <cstdint>

namespace mir
{
class Scene;
}

namespace MirEngine::Rendering
{

class RenderContext;

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

protected:
    Renderer() = default;
};

} // namespace MirEngine::Rendering
