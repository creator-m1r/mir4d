#pragma once

#include "SimulationTypes.hpp"

#include <cstdint>
#include <string>

namespace mir
{

enum class SimulationFieldType : std::uint8_t
{
    Pressure,
    Temperature,
    Velocity,
    FlowRate,
    Density,
    Viscosity,
    PH,
    Concentration,
    HeatFlux,
    Drag,
    Lift
};

struct SimulationField
{
    SimulationFieldType type{SimulationFieldType::Temperature};
    std::string name{};
    std::string unit{};
    Scalar minimum{0.0};
    Scalar maximum{1.0};
    bool visible{true};
};

}
