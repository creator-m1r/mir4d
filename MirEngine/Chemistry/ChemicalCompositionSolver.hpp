#pragma once

#include "ChemicalReaction.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace mir
{

struct ChemicalCompositionDelta
{
    std::unordered_map<std::uint32_t, float> molarDelta;
    float extent{0.0F};
};

class ChemicalCompositionSolver
{
public:
    [[nodiscard]] ChemicalCompositionDelta calculateDelta(
        const ChemicalReaction& reaction,
        const MaterialComposition& composition,
        float requestedExtent) const
    {
        ChemicalCompositionDelta result{};
        if (!reaction.enabled || requestedExtent <= 0.0F)
            return result;

        float limitingExtent = requestedExtent;

        for (const auto& participant : reaction.reactants)
        {
            const float coefficient = std::max(0.000001F, participant.coefficient);
            const float available = findMolarFraction(
                composition,
                participant.componentId);
            limitingExtent = std::min(limitingExtent, available / coefficient);
        }

        result.extent = std::max(0.0F, limitingExtent);

        for (const auto& participant : reaction.reactants)
        {
            result.molarDelta[participant.componentId] -=
                participant.coefficient * result.extent;
        }

        for (const auto& participant : reaction.products)
        {
            result.molarDelta[participant.componentId] +=
                participant.coefficient * result.extent;
        }

        return result;
    }

    void apply(
        MaterialComposition& composition,
        const ChemicalCompositionDelta& delta) const
    {
        for (const auto& [componentId, amount] : delta.molarDelta)
        {
            if (composition.adjustMolarFraction(componentId, amount))
                continue;

            if (amount <= 0.0F)
                continue;

            composition.addComponent({
                componentId,
                "Component_" + std::to_string(componentId),
                0.0F,
                amount});
        }
    }

private:
    [[nodiscard]] static float findMolarFraction(
        const MaterialComposition& composition,
        std::uint32_t componentId) noexcept
    {
        for (const auto& component : composition.components())
        {
            if (component.id == componentId)
                return std::max(0.0F, component.molarFraction);
        }
        return 0.0F;
    }
};

}