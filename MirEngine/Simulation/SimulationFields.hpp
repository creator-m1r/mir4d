#pragma once

#include "FieldTypes.hpp"

#include <vector>

namespace mir
{

class SimulationFields
{
public:
    SimulationFields()
    {
        fields_.push_back({SimulationFieldType::Pressure, "Pressure", "Pa", 0.0, 101325.0, false});
        fields_.push_back({SimulationFieldType::Temperature, "Temperature", "K", 273.15, 373.15, false});
        fields_.push_back({SimulationFieldType::Velocity, "Velocity", "m/s", 0.0, 100.0, false});
        fields_.push_back({SimulationFieldType::FlowRate, "Flow rate", "m³/s", 0.0, 1.0, false});
        fields_.push_back({SimulationFieldType::Density, "Density", "kg/m³", 0.0, 2000.0, false});
        fields_.push_back({SimulationFieldType::Viscosity, "Viscosity", "Pa·s", 0.0, 10.0, false});
        fields_.push_back({SimulationFieldType::PH, "pH", "", 0.0, 14.0, false});
        fields_.push_back({SimulationFieldType::Concentration, "Concentration", "%", 0.0, 100.0, false});
        fields_.push_back({SimulationFieldType::HeatFlux, "Heat flux", "W/m²", 0.0, 10000.0, false});
        fields_.push_back({SimulationFieldType::Drag, "Drag", "N", 0.0, 10000.0, false});
        fields_.push_back({SimulationFieldType::Lift, "Lift", "N", 0.0, 10000.0, false});
    }

    [[nodiscard]] const std::vector<SimulationField>& all() const noexcept { return fields_; }

    bool setVisible(SimulationFieldType type, bool visible) noexcept
    {
        for (auto& field : fields_)
        {
            if (field.type == type)
            {
                field.visible = visible;
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool isVisible(SimulationFieldType type) const noexcept
    {
        for (const auto& field : fields_)
            if (field.type == type)
                return field.visible;
        return false;
    }

private:
    std::vector<SimulationField> fields_{};
};

}
