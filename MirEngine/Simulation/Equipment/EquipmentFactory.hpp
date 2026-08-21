#pragma once

#include "EquipmentModel.hpp"

#include <cstdint>
#include <string_view>

namespace mir
{

enum class EquipmentType : std::uint8_t
{
    Motor,
    Pump,
    Pipe,
    Tank,
    Valve,
    HeatExchanger,
    Fan,
    Separator
};

class EquipmentFactory
{
public:
    [[nodiscard]] static EquipmentModel create(
        EquipmentType type,
        std::uint64_t id,
        std::string_view name)
    {
        EquipmentModel model{};
        model.id = id;
        model.name = std::string(name);
        model.type = typeName(type);

        switch (type)
        {
        case EquipmentType::Motor:
            addParameter(model, "speed", 1450.0, "rpm");
            addParameter(model, "power", 2.2, "kW");
            addPort(model, 1, "MechanicalOutput", EquipmentPortType::MechanicalOutput);
            addPort(model, 2, "ControlInput", EquipmentPortType::ControlInput);
            break;

        case EquipmentType::Pump:
            addParameter(model, "speed", 1450.0, "rpm");
            addParameter(model, "pressure", 300000.0, "Pa");
            addParameter(model, "flow", 1.0, "kg/s");
            addPort(model, 1, "MechanicalInput", EquipmentPortType::MechanicalInput);
            addPort(model, 2, "FluidInput", EquipmentPortType::FluidInput);
            addPort(model, 3, "FluidOutput", EquipmentPortType::FluidOutput);
            break;

        case EquipmentType::Pipe:
            addParameter(model, "diameter", 0.05, "m");
            addParameter(model, "length", 1.0, "m");
            addPort(model, 1, "FluidInput", EquipmentPortType::FluidInput);
            addPort(model, 2, "FluidOutput", EquipmentPortType::FluidOutput);
            break;

        case EquipmentType::Tank:
            addParameter(model, "volume", 1.0, "m3");
            addParameter(model, "level", 0.0, "m3");
            addPort(model, 1, "FluidInput", EquipmentPortType::FluidInput);
            addPort(model, 2, "FluidOutput", EquipmentPortType::FluidOutput);
            addPort(model, 3, "ControlInput", EquipmentPortType::ControlInput);
            break;

        case EquipmentType::Valve:
            addParameter(model, "opening", 1.0, "ratio");
            addPort(model, 1, "FluidInput", EquipmentPortType::FluidInput);
            addPort(model, 2, "FluidOutput", EquipmentPortType::FluidOutput);
            addPort(model, 3, "ControlInput", EquipmentPortType::ControlInput);
            break;

        case EquipmentType::HeatExchanger:
            addParameter(model, "area", 10.0, "m2");
            addParameter(model, "efficiency", 0.8, "ratio");
            addPort(model, 1, "FluidInput", EquipmentPortType::FluidInput);
            addPort(model, 2, "FluidOutput", EquipmentPortType::FluidOutput);
            addPort(model, 3, "ThermalInput", EquipmentPortType::ThermalInput);
            addPort(model, 4, "ThermalOutput", EquipmentPortType::ThermalOutput);
            break;

        case EquipmentType::Fan:
            addParameter(model, "speed", 1200.0, "rpm");
            addParameter(model, "flow", 2.0, "m3/s");
            addPort(model, 1, "MechanicalInput", EquipmentPortType::MechanicalInput);
            addPort(model, 2, "FluidOutput", EquipmentPortType::FluidOutput);
            break;

        case EquipmentType::Separator:
            addParameter(model, "speed", 6000.0, "rpm");
            addParameter(model, "capacity", 1.0, "kg/s");
            addPort(model, 1, "FluidInput", EquipmentPortType::FluidInput);
            addPort(model, 2, "FluidOutput", EquipmentPortType::FluidOutput);
            addPort(model, 3, "ControlInput", EquipmentPortType::ControlInput);
            break;
        }

        addCommand(model, "START");
        addCommand(model, "STOP");
        return model;
    }

    static bool addToStore(
        EquipmentModelStore& store,
        EquipmentType type,
        std::uint64_t id,
        std::string_view name)
    {
        return store.add(create(type, id, name));
    }

private:
    [[nodiscard]] static constexpr std::string_view typeName(EquipmentType type) noexcept
    {
        switch (type)
        {
        case EquipmentType::Motor: return "Motor";
        case EquipmentType::Pump: return "Pump";
        case EquipmentType::Pipe: return "Pipe";
        case EquipmentType::Tank: return "Tank";
        case EquipmentType::Valve: return "Valve";
        case EquipmentType::HeatExchanger: return "HeatExchanger";
        case EquipmentType::Fan: return "Fan";
        case EquipmentType::Separator: return "Separator";
        }
        return "Equipment";
    }

    static void addPort(
        EquipmentModel& model,
        std::uint64_t id,
        std::string_view name,
        EquipmentPortType type)
    {
        model.ports.push_back({id, std::string(name), type, true});
    }

    static void addParameter(
        EquipmentModel& model,
        std::string_view name,
        Scalar value,
        std::string_view unit)
    {
        model.parameters.push_back({std::string(name), value, std::string(unit)});
    }

    static void addCommand(EquipmentModel& model, std::string_view name)
    {
        model.commands.push_back({std::string(name), 0.0, true});
    }
};

}
