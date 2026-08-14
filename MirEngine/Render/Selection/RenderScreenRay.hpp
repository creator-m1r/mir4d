#pragma once

#include "RenderRay.hpp"

namespace mir
{

class RenderScreenRayBuilder
{
public:
    [[nodiscard]] static RenderRay fromScreen(
        double screenX,
        double screenY,
        double viewportWidth,
        double viewportHeight,
        const RenderCamera& camera) noexcept;
};

} // namespace mir
