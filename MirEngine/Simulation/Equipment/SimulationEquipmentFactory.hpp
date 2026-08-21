#pragma once

#include "../SimulationEquipment.hpp"

namespace mir
{

class SimulationEquipmentFactory
{
public:
    [[nodiscard]] static SimulationEquipment pump(const std::string& id, const std::string& name)
    {
        SimulationEquipment item{id, name, SimulationEquipmentType::Pump};
        item.ports = {
            {id + ".in", "Inlet", SimulationPortDirection::Input, SimulationPortType::Material, 1.0, {}},
            {id + ".out", "Outlet", SimulationPortDirection::Output, SimulationPortType::Material, 1.0, {}}
        };
        return item;
    }

    [[nodiscard]] static SimulationEquipment pipe(const std::string& id, const std::string& name)
    {
        SimulationEquipment item{id, name, SimulationEquipmentType::Pipe};
        item.ports = {
            {id + ".in", "Inlet", SimulationPortDirection::Input, SimulationPortType::Material, 1.0, {}},
            {id + ".out", "Outlet", SimulationPortDirection::Output, SimulationPortType::Material, 1.0, {}}
        };
        return item;
    }

    [[nodiscard]] static SimulationEquipment tank(const std::string& id, const std::string& name)
    {
        SimulationEquipment item{id, name, SimulationEquipmentType::Tank};
        item.ports = {
            {id + ".in", "Inlet", SimulationPortDirection::Input, SimulationPortType::Material, 1.0, {}},
            {id + ".out", "Outlet", SimulationPortDirection::Output, SimulationPortType::Material, 1.0, {}}
        };
        return item;
    }

    [[nodiscard]] static SimulationEquipment heatExchanger(const std::string& id, const std::string& name)
    {
        SimulationEquipment item{id, name, SimulationEquipmentType::HeatExchanger};
        item.ports = {
            {id + ".process.in", "Process In", SimulationPortDirection::Input, SimulationPortType::Material, 1.0, {}},
            {id + ".process.out", "Process Out", SimulationPortDirection::Output, SimulationPortType::Material, 1.0, {}},
            {id + ".coolant.in", "Coolant In", SimulationPortDirection::Input, SimulationPortType::Material, 1.0, {}},
            {id + ".coolant.out", "Coolant Out", SimulationPortDirection::Output, SimulationPortType::Material, 1.0, {}}
        };
        return item;
    }
};

} // namespace mir
