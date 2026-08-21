#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mir
{

using Scalar = double;

struct MaterialComponent
{
    std::string name;
    Scalar fraction{0.0};
};

struct MaterialProperties
{
    std::uint64_t id{0};
    std::string name;
    Scalar density{1000.0};
    Scalar viscosity{0.001};
    Scalar specificHeat{4181.0};
    Scalar thermalConductivity{0.6};
    Scalar referenceTemperature{293.15};
    std::vector<MaterialComponent> composition;

    void normalizeComposition() noexcept
    {
        Scalar sum = 0.0;
        for (const auto& component : composition)
            sum += std::max(0.0, component.fraction);

        if (sum <= 0.0)
            return;

        for (auto& component : composition)
            component.fraction = std::max(0.0, component.fraction) / sum;
    }
};

class MaterialPropertiesStore
{
public:
    void clear() noexcept
    {
        materials_.clear();
    }

    void set(MaterialProperties material)
    {
        material.density = std::max(0.0, material.density);
        material.viscosity = std::max(0.0, material.viscosity);
        material.specificHeat = std::max(0.0, material.specificHeat);
        material.thermalConductivity = std::max(0.0, material.thermalConductivity);
        material.normalizeComposition();
        materials_[material.id] = std::move(material);
    }

    [[nodiscard]] const MaterialProperties* get(std::uint64_t id) const noexcept
    {
        const auto it = materials_.find(id);
        return it == materials_.end() ? nullptr : &it->second;
    }

private:
    std::unordered_map<std::uint64_t, MaterialProperties> materials_;
};

}
