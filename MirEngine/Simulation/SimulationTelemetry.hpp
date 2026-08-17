#pragma once

#include "SimulationTypes.hpp"

#include <cstddef>

namespace mir
{

struct SimulationTelemetry
{
    Scalar time{0.0};
    std::size_t objects{0};
    Scalar totalFlowRate{0.0};
    Scalar averageTemperature{293.15};
    Scalar averagePressure{101325.0};
    Scalar averagePH{7.0};
    Scalar totalDrag{0.0};
    Scalar maxTemperature{293.15};
    Scalar minTemperature{293.15};
    Scalar maxPressure{101325.0};
    Scalar maxStress{0.0};
    Scalar maxVelocity{0.0};
    Scalar maxAcoustic{0.0};
    bool running{false};
};

} // namespace mir
