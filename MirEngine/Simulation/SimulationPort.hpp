#pragma once

#include "SimulationMaterial.hpp"

#include <string>

namespace mir
{

enum class SimulationPortDirection
{
    Input,
    Output
};

enum class SimulationPortType
{
    Material,
    Energy,
    Motion,
    Signal,
    Air
};

struct SimulationPort
{
    std::string id{};
    std::string name{};
    SimulationPortDirection direction{SimulationPortDirection::Input};
    SimulationPortType type{SimulationPortType::Material};
    Scalar capacity{1.0};
    SimulationMaterial material{};
};

}
