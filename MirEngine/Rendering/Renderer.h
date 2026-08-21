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

struct HandSkeletonStyle;

class Renderer
{
public:
    virtual ~Renderer() = default;

    virtual bool initialize() = 0;

    virtual void render(mir::Scene& scene,
                        RenderContext& context) = 0;

    virtual void resize(std::uint32_t width, std::uint32_t height) = 0;

    virtual void setObjectMaterial(std::uint64_t objectId,
                                   std::int32_t materialId) noexcept
    {
        (void)objectId;
        (void)materialId;
    }

    virtual void setHandSkeletonStyle(const HandSkeletonStyle& ) noexcept {}

    virtual void setHandSkeletonTopology(const std::vector<std::pair<int, int>>& ) noexcept {}

protected:
    Renderer() = default;
};

}
