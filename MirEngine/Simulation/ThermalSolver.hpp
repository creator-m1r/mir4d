#pragma once

#include "ProcessTypes.hpp"

#include <algorithm>

namespace mir
{

enum class ThermalQuality
{
    Fast,
    Balanced,
    High
};

struct ThermalNodeState
{
    Scalar temperature{293.15};
    Scalar heatCapacity{1000.0};
    Scalar conductivity{0.5};
    Scalar mass{1.0};
};

struct ThermalBoundary
{
    Scalar ambientTemperature{293.15};
    Scalar convection{5.0};
};

class ThermalSolver
{
public:
    void setQuality(ThermalQuality quality) noexcept { quality_ = quality; }
    [[nodiscard]] ThermalQuality quality() const noexcept { return quality_; }

    void solve(
        ThermalNodeState& fluid,
        ThermalNodeState& wall,
        const ThermalBoundary& boundary,
        Scalar deltaSeconds) const noexcept
    {
        if (deltaSeconds <= 0.0)
            return;

        const Scalar fluidCapacity = capacity(fluid);
        const Scalar wallCapacity = capacity(wall);
        const Scalar exchange = exchangeFactor();

        const Scalar fluidToWall = (fluid.temperature - wall.temperature)
            * std::max(0.0, fluid.conductivity)
            * exchange * deltaSeconds;

        const Scalar wallToAmbient = (wall.temperature - boundary.ambientTemperature)
            * std::max(0.0, boundary.convection)
            * 0.001 * exchange * deltaSeconds;

        fluid.temperature -= fluidToWall / fluidCapacity;
        wall.temperature += fluidToWall / wallCapacity;
        wall.temperature -= wallToAmbient / wallCapacity;

        fluid.temperature = std::max(0.0, fluid.temperature);
        wall.temperature = std::max(0.0, wall.temperature);
    }

private:
    [[nodiscard]] static Scalar capacity(const ThermalNodeState& state) noexcept
    {
        return std::max(0.001, state.heatCapacity * std::max(0.001, state.mass));
    }

    [[nodiscard]] Scalar exchangeFactor() const noexcept
    {
        switch (quality_)
        {
            case ThermalQuality::Fast:     return 1.0;
            case ThermalQuality::Balanced: return 0.5;
            case ThermalQuality::High:     return 0.25;
        }
        return 1.0;
    }

    ThermalQuality quality_{ThermalQuality::Fast};
};

}
