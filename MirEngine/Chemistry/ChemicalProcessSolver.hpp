#pragma once

#include "ChemicalReaction.hpp"
#include "../Materials/MaterialStateField.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace mir
{

struct ChemicalProcessResult
{
    float reactionProgress{0.0F};
    float heatGeneratedJPerMol{0.0F};
    float temperatureDeltaK{0.0F};
    bool stateChanged{false};
};

class ChemicalProcessSolver
{
public:
    [[nodiscard]] ChemicalProcessResult step(
        const ChemicalReaction& reaction,
        MaterialComposition& composition,
        MaterialStateField& state,
        float deltaTimeSeconds) const noexcept
    {
        if (deltaTimeSeconds <= 0.0F || !reaction.enabled)
            return {};

        const ChemicalReactionEvaluator evaluator{};
        const float progress = evaluator.progress(reaction, composition);
        if (progress <= 0.0F)
            return {};

        const float boundedDt = std::clamp(deltaTimeSeconds, 0.0F, 1.0F);
        const float reactionAmount = std::clamp(progress * boundedDt, 0.0F, 1.0F);
        const float heat = reactionAmount * reaction.heatJPerMol;

        auto& materialState = state.state();
        const float heatCapacity = std::max(
            0.001F,
            materialState.specificHeatJKgK * std::max(0.001F, materialState.densityKgM3));
        const float temperatureDelta = heat / heatCapacity;

        materialState.temperatureK = std::max(
            0.0F,
            materialState.temperatureK + temperatureDelta);

        return {
            reactionAmount,
            heat,
            temperatureDelta,
            true};
    }
};

} // namespace mir
