#pragma once

namespace MirEngine {
namespace Rendering {

struct RenderViewportSize
{
    int width{1};
    int height{1};
};

struct RenderViewportInput
{
    double orbitDeltaX{0.0};
    double orbitDeltaY{0.0};
    double panDeltaX{0.0};
    double panDeltaY{0.0};
    double zoomDelta{0.0};
    bool fitAll{false};
};

} // namespace Rendering
} // namespace MirEngine
