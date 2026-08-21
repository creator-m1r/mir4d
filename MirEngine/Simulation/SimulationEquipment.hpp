#pragma once

#include "SimulationPort.hpp"

#include <string>
#include <vector>

namespace mir
{

enum class SimulationEquipmentType
{
    Generic,
    Pump,
    Motor,
    Pipe,
    Valve,
    Tank,
    HeatExchanger,
    Reactor,
    Separator,
    Fan,
    Compressor
};

struct SimulationEquipment
{
    std::string id{};
    std::string name{};
    SimulationEquipmentType type{SimulationEquipmentType::Generic};
    std::vector<SimulationPort> ports{};

    bool enabled{true};
    bool running{false};
    Scalar efficiency{1.0};
    Scalar power{0.0};
    Scalar speed{0.0};
    Scalar flow{0.0};
    Scalar pressure{101325.0};
    Scalar temperature{293.15};

    [[nodiscard]] SimulationPort* port(const std::string& portId) noexcept
    {
        for (auto& item : ports)
            if (item.id == portId)
                return &item;
        return nullptr;
    }

    [[nodiscard]] const SimulationPort* port(const std::string& portId) const noexcept
    {
        for (const auto& item : ports)
            if (item.id == portId)
                return &item;
        return nullptr;
    }
};

} // namespace mir
