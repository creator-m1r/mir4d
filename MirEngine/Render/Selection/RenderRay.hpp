#pragma once

#include "../RenderCamera.hpp"

namespace mir
{

struct RenderRay
{
    RenderVec3 origin{};
    RenderVec3 direction{0.0, 0.0, -1.0};
};

} // namespace mir
