#pragma once

#include "SimulationMaterial.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace mir
{

enum class ChemicalQuality
{
    Fast,
    Balanced,
    High
};

struct ChemicalComponent
{
    std::string id{};
    std::string formula{};
    Scalar concentration{0.0};
};

struct ChemicalReaction
{
    std::string id{};
    std::vector<std::string> reactants{};
    std::vector<std::string> products{};
    Scalar rate{0.0};
    Scalar activationTemperature{273.15};
};

struct ChemicalState
{
    std::vector<ChemicalComponent> components{};
    Scalar pH{7.0};
    Scalar temperature{293.15};
    Scalar pressure{101325.0};
};

class ChemicalSolver
{
public:
    void setQuality(ChemicalQuality quality) noexcept { quality_ = quality; }
    [[nodiscard]] ChemicalQuality quality() const noexcept { return quality_; }

    void solve(ChemicalState& state, const std::vector<ChemicalReaction>& reactions, Scalar deltaSeconds) const noexcept
    {
        if (deltaSeconds <= 0.0)
            return;

        const Scalar factor = responseFactor();

        for (const auto& reaction : reactions)
        {
            if (reaction.rate <= 0.0 || state.temperature < reaction.activationTemperature)
                continue;

            const Scalar change = reaction.rate * deltaSeconds * factor;

            for (const auto& id : reaction.reactants)
                consume(state, id, change);

            for (const auto& id : reaction.products)
                produce(state, id, change);
        }

        state.pH = std::clamp(state.pH, 0.0, 14.0);
    }

    [[nodiscard]] static SimulationMaterial toMaterial(const ChemicalState& state, const std::string& id, const std::string& name)
    {
        SimulationMaterial material{};
        material.id = id;
        material.name = name;
        material.temperature = state.temperature;
        material.pressure = state.pressure;
        material.pH = state.pH;
        material.concentration = state.components.empty() ? 0.0 : state.components.front().concentration;
        return material;
    }

private:
    static void consume(ChemicalState& state, const std::string& id, Scalar amount) noexcept
    {
        for (auto& component : state.components)
        {
            if (component.id == id)
            {
                component.concentration = std::max(0.0, component.concentration - amount);
                return;
            }
        }
    }

    static void produce(ChemicalState& state, const std::string& id, Scalar amount)
    {
        for (auto& component : state.components)
        {
            if (component.id == id)
            {
                component.concentration += amount;
                return;
            }
        }

        state.components.push_back({id, {}, amount});
    }

    [[nodiscard]] Scalar responseFactor() const noexcept
    {
        switch (quality_)
        {
            case ChemicalQuality::Fast:     return 1.0;
            case ChemicalQuality::Balanced: return 0.5;
            case ChemicalQuality::High:     return 0.25;
        }
        return 1.0;
    }

    ChemicalQuality quality_{ChemicalQuality::Fast};
};

} // namespace mir
