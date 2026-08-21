#pragma once

#include "SimulationField.hpp"

#include <algorithm>

namespace mir
{

struct SimulationFieldVisualSettings
{
    bool enabled{false};
    bool showLegend{true};
    bool showValues{true};
    Scalar opacity{0.75};
    Scalar minValue{0.0};
    Scalar maxValue{1.0};
};

class SimulationFieldPalette
{
public:
    [[nodiscard]] static Scalar normalized(
        const SimulationFieldSample& sample,
        const SimulationFieldVisualSettings& settings) noexcept
    {
        const Scalar minValue = std::min(settings.minValue, settings.maxValue);
        const Scalar maxValue = std::max(settings.minValue, settings.maxValue);
        const Scalar range = maxValue - minValue;
        if (range <= 0.0)
            return 0.0;

        return std::clamp((sample.value - minValue) / range, 0.0, 1.0);
    }
};

} // namespace mir
