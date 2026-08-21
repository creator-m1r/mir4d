#pragma once

#include "Camera.hpp"
#include "ViewportController.hpp"
#include "MirEngine/Interaction/RayPicker.hpp"
#include "MirEngine/Interaction/SelectionState.hpp"

namespace mir
{

struct ViewportState
{
    Camera camera{};
    ViewportController controller{&camera};
    SelectionState selection{};

    mir4d::ObjectId hoveredObjectId{mir4d::InvalidObjectId};

    std::uint32_t width{1};
    std::uint32_t height{1};

    void resize(std::uint32_t newWidth, std::uint32_t newHeight) noexcept
    {
        width = newWidth == 0 ? 1 : newWidth;
        height = newHeight == 0 ? 1 : newHeight;
        camera.setAspect(static_cast<Scalar>(width) / static_cast<Scalar>(height));
    }

    [[nodiscard]] PickResult pick(const Scene& scene,
                                  Scalar x,
                                  Scalar y) const noexcept
    {
        return RayPicker::pick(scene, camera, x, y, width, height);
    }
};

}
