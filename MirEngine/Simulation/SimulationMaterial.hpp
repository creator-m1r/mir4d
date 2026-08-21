#pragma once

#include "SimulationTypes.hpp"

#include <string>

namespace mir
{

struct SimulationMaterial
{
    std::string id{};
    std::string name{};
    std::string formula{};
    Scalar density{1000.0};
    Scalar viscosity{0.001};
    Scalar temperature{293.15};
    Scalar pressure{101325.0};
    Scalar pH{7.0};
    Scalar concentration{100.0};
    bool fluid{true};
};

}
