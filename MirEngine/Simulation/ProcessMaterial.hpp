#pragma once

#include "SimulationTypes.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace mir
{

struct MaterialComponent
{
    std::string id;
    std::string formula;
    Scalar fraction{0.0};
};

struct ProcessMaterial
{
    std::string id;
    std::string name;
    std::vector<MaterialComponent> components;
    FluidState fluid{};
    ChemicalState chemistry{};
    Scalar mass{0.0};

    void normalizeFractions() noexcept
    {
        Scalar total = 0.0;
        for (const auto& component : components)
            total += std::max(0.0, component.fraction);

        if (total <= 0.0)
            return;

        for (auto& component : components)
            component.fraction = std::max(0.0, component.fraction) / total;
    }
};

} // namespace mir
