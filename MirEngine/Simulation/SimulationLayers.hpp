#pragma once

#include "SimulationTypes.hpp"

#include <array>

namespace mir
{

class SimulationLayers
{
public:
    void setVisible(SimulationLayer layer, bool visible) noexcept
    {
        visible_[static_cast<std::size_t>(layer)] = visible;
    }

    [[nodiscard]] bool isVisible(SimulationLayer layer) const noexcept
    {
        return visible_[static_cast<std::size_t>(layer)];
    }

    void showAll() noexcept { visible_.fill(true); }
    void hideAll() noexcept { visible_.fill(false); }

private:
    std::array<bool, 6> visible_{true, true, true, true, true, true};
};

}
