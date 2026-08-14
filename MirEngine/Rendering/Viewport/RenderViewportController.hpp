#pragma once

#include "RenderViewport.hpp"
#include "../Resources/RenderBounds.hpp"

namespace MirEngine {
namespace Rendering {

class RenderViewportController
{
public:
    explicit RenderViewportController(RenderViewport& viewport) noexcept
        : viewport_(viewport)
    {
    }

    void orbit(double deltaX, double deltaY) noexcept
    {
        RenderViewportInput input{};
        input.orbitDeltaX = deltaX;
        input.orbitDeltaY = deltaY;
        viewport_.processInput(input);
    }

    void pan(double deltaX, double deltaY) noexcept
    {
        RenderViewportInput input{};
        input.panDeltaX = deltaX;
        input.panDeltaY = deltaY;
        viewport_.processInput(input);
    }

    void zoom(double delta) noexcept
    {
        RenderViewportInput input{};
        input.zoomDelta = delta;
        viewport_.processInput(input);
    }

    void fitAll(const RenderBounds& bounds) noexcept
    {
        if (!bounds.valid())
            return;

        auto& camera = viewport_.camera();
        camera.setTarget(bounds.center());
        camera.setDistance(std::max(bounds.radius() * 2.5, 1e-3));
    }

private:
    RenderViewport& viewport_;
};

} // namespace Rendering
} // namespace MirEngine
