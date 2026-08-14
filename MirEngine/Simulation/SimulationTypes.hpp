#pragma once

#include "../Core/Types/Scalar.hpp"
#include "../Math/Vector/Vector3.hpp"
#include "../World/WorldObject.hpp"

#include <cstdint>

namespace mir
{

enum class SimulationLayer : std::uint8_t
{
    Mechanics,
    Fluid,
    Thermal,
    Chemistry,
    Aerodynamics,
    Acoustics
};

struct FluidState
{
    Scalar pressure{101325.0};
    Scalar temperature{293.15};
    Scalar density{1000.0};
    Scalar viscosity{0.001};
    Vector3 velocity{};
    Scalar flowRate{0.0};
};

struct ThermalState
{
    Scalar temperature{293.15};
    Scalar heatGeneration{0.0};
    Scalar heatFlux{0.0};
};

struct ChemicalState
{
    Scalar concentration{0.0};
    Scalar reactionRate{0.0};
    Scalar pH{7.0};
};

struct AerodynamicState
{
    Vector3 airVelocity{};
    Scalar pressure{101325.0};
    Scalar drag{0.0};
    Scalar lift{0.0};
};

struct SimulationSettings
{
    bool running{false};
    Scalar time{0.0};
    Scalar timeScale{1.0};
    bool mechanics{true};
    bool fluid{true};
    bool thermal{true};
    bool chemistry{true};
    bool aerodynamics{true};
    bool acoustics{true};
};

} // namespace mir
