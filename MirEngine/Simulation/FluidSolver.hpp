#pragma once

#include "ProcessTypes.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{

enum class SimulationQuality
{
    Fast,
    Balanced,
    High
};

struct FluidNodeState
{
    Scalar pressure{101325.0};
    Scalar temperature{293.15};
    Scalar flow{0.0};
    Scalar velocity{0.0};
    Scalar density{1000.0};
    Scalar viscosity{0.001};
};

struct FluidConnectionState
{
    Scalar capacity{1.0};
    Scalar resistance{0.0};
};

class FluidSolver
{
public:
    void setQuality(SimulationQuality quality) noexcept { quality_ = quality; }
    [[nodiscard]] SimulationQuality quality() const noexcept { return quality_; }

    void solve(
        FluidNodeState& source,
        FluidNodeState& target,
        const FluidConnectionState& connection,
        Scalar deltaSeconds) const noexcept
    {
        if (deltaSeconds <= 0.0 || connection.capacity <= 0.0)
            return;

        const Scalar pressureDelta = source.pressure - target.pressure;
        const Scalar resistance = std::max(0.0, connection.resistance);
        const Scalar response = responseFactor();

        const Scalar pressureFlow = pressureDelta / std::max(1.0, 1.0 + resistance);
        const Scalar requestedFlow = std::max(0.0, source.flow + pressureFlow * deltaSeconds * 0.001);
        const Scalar transferred = std::min(requestedFlow, connection.capacity) * response;

        target.flow += transferred * deltaSeconds;
        source.flow = std::max(0.0, source.flow - transferred * deltaSeconds);

        target.pressure += pressureDelta * 0.02 * response;
        target.temperature += (source.temperature - target.temperature) * 0.02 * response;

        target.density = std::max(0.001, target.density);
        target.velocity = transferred / target.density;
        target.viscosity = std::max(0.000001, target.viscosity);
    }

private:
    [[nodiscard]] Scalar responseFactor() const noexcept
    {
        switch (quality_)
        {
            case SimulationQuality::Fast:     return 1.0;
            case SimulationQuality::Balanced: return 0.5;
            case SimulationQuality::High:     return 0.25;
        }
        return 1.0;
    }

    SimulationQuality quality_{SimulationQuality::Fast};
};

} // namespace mir
