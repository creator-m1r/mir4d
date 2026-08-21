#pragma once

#include "MaterialStateField.hpp"
#include "../Chemistry/MaterialComposition.hpp"

#include <algorithm>

namespace mir
{

class MaterialStateBinding
{
public:
    void bind(MaterialStateField& state, const MaterialComposition& composition) const noexcept
    {
        auto& physical = state.state();
        physical.temperatureK = composition.temperatureK();
        physical.pressurePa = composition.pressurePa();
        physical.phaseFraction = composition.totalMassFraction();
        physical.phaseFraction = std::clamp(physical.phaseFraction, 0.0F, 1.0F);
    }
};

}
