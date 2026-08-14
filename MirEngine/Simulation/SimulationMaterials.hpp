#pragma once

#include "SimulationMaterial.hpp"

#include <string>
#include <unordered_map>

namespace mir
{

class SimulationMaterials
{
public:
    void registerMaterial(const SimulationMaterial& material)
    {
        materials_[material.id] = material;
    }

    [[nodiscard]] SimulationMaterial* find(const std::string& id) noexcept
    {
        const auto it = materials_.find(id);
        return it == materials_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const SimulationMaterial* find(const std::string& id) const noexcept
    {
        const auto it = materials_.find(id);
        return it == materials_.end() ? nullptr : &it->second;
    }

private:
    std::unordered_map<std::string, SimulationMaterial> materials_{};
};

} // namespace mir
