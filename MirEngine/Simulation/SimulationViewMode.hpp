#pragma once

#include "SimulationField.hpp"

#include <cstdint>

namespace mir
{

enum class SimulationViewMode
{
    Realistic,
    FlowRate,
    Pressure,
    Temperature,
    ChemicalConcentration,
    AirVelocity,
    CombinedFields
};

struct SimulationViewSettings
{
    SimulationViewMode mode{SimulationViewMode::Realistic};
    bool showLegend{true};
    bool showValues{true};
    bool showParticles{true};
    bool showVectors{true};
    bool showInternalLayers{false};
    bool isolateSelectedLayer{false};
    Scalar opacity{0.75};
};

class SimulationViewController
{
public:
    void setMode(SimulationViewMode mode) noexcept
    {
        settings_.mode = mode;
    }

    [[nodiscard]] SimulationViewMode mode() const noexcept
    {
        return settings_.mode;
    }

    void setSettings(SimulationViewSettings settings) noexcept
    {
        settings_.opacity = settings.opacity < 0.0 ? 0.0 :
            (settings.opacity > 1.0 ? 1.0 : settings.opacity);
        settings_ = settings;
    }

    [[nodiscard]] const SimulationViewSettings& settings() const noexcept
    {
        return settings_;
    }

private:
    SimulationViewSettings settings_{};
};

} // namespace mir
