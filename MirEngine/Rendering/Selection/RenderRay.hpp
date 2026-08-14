#pragma once

#include "../Camera/RenderCamera.hpp"

namespace MirEngine {
namespace Rendering {

struct RenderRay
{
    RenderVec3 origin{};
    RenderVec3 direction{0.0, 0.0, -1.0};
};

} // namespace Rendering
} // namespace MirEngine
