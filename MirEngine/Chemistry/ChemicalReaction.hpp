#pragma once

#include "MaterialComposition.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace mir
{

struct ReactionParticipant
{
    std::uint32_t componentId{0};
    float coefficient{1.0F};
};

struct ChemicalReaction
{
    std::uint32_t id{0};
    std::vector<ReactionParticipant> reactants;
    std::vector<ReactionParticipant> products;
    float activationEnergyJPerMol{0.0F};
    float heatJPerMol{0.0F};
    float rateConstant{0.0F};
    bool reversible{false};
    bool enabled{true};
};

class ChemicalReactionEvaluator
{
public:
    [[nodiscard]] float progress(
        const ChemicalReaction& reaction,
        const MaterialComposition& composition) const noexcept
    {
        if (!reaction.enabled || reaction.reactants.empty())
            return 0.0F;

        float limiting = 1.0F;
        for (const auto& participant : reaction.reactants)
        {
            const float coefficient = std::max(0.000001F, participant.coefficient);
            float fraction = 0.0F;

            for (const auto& component : composition.components())
            {
                if (component.id == participant.componentId)
                {
                    fraction = std::max(0.0F, component.molarFraction);
                    break;
                }
            }

            limiting = std::min(limiting, fraction / coefficient);
        }

        return std::clamp(limiting * std::max(0.0F, reaction.rateConstant), 0.0F, 1.0F);
    }
};

}
