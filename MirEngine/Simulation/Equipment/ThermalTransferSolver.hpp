#pragma once

#include "EquipmentSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string_view>

namespace mir
{

struct ThermalTransferResult
{
    std::size_t elementsProcessed{0};
    Scalar maxTemperature{0.0};
    Scalar totalHeatTransfer{0.0};
};

class ThermalTransferSolver
{
public:
    [[nodiscard]] ThermalTransferResult step(
        EquipmentModelStore& equipment,
        const EquipmentConnectionStore& connections,
        const EquipmentSystem& system,
        Scalar deltaTime) const noexcept
    {
        ThermalTransferResult result{};
        const Scalar dt = std::max(0.0, deltaTime);
        if (!system.enabled || !system.running || dt <= 0.0)
            return result;

        for (const auto equipmentId : system.equipmentIds)
        {
            auto* model = equipment.get(equipmentId);
            if (!model || !model->enabled)
                continue;

            updateElement(*model, dt, result);
            ++result.elementsProcessed;
        }

        for (const auto equipmentId : system.equipmentIds)
        {
            for (const auto& connection : connections.forEquipment(equipmentId))
            {
                if (!connection.enabled ||
                    connection.process != EquipmentConnectionProcess::Thermal)
                    continue;

                auto* source = equipment.get(connection.sourceEquipmentId);
                auto* target = equipment.get(connection.targetEquipmentId);
                if (!source || !target || !source->enabled || !target->enabled)
                    continue;

                const Scalar before = target->state.temperature;
                const Scalar coefficient = std::clamp(dt * 0.5, 0.0, 1.0);
                target->state.temperature +=
                    (source->state.temperature - target->state.temperature) * coefficient;
                result.totalHeatTransfer +=
                    std::abs(target->state.temperature - before);
                result.maxTemperature =
                    std::max(result.maxTemperature, target->state.temperature);
            }
        }

        return result;
    }

private:
    static void updateElement(
        EquipmentModel& model,
        Scalar dt,
        ThermalTransferResult& result) noexcept
    {
        const Scalar ambient = parameter(model, "ambientTemperature").value_or(293.15);
        const Scalar conductivity = std::max(
            parameter(model, "thermalConductivity").value_or(1.0), 0.0);
        const Scalar thermalMass = std::max(
            parameter(model, "thermalMass").value_or(1000.0), 1.0);

        const Scalar relaxation = std::clamp(
            dt * conductivity / thermalMass,
            0.0,
            1.0);

        const Scalar before = model.state.temperature;
        model.state.temperature +=
            (ambient - model.state.temperature) * relaxation;

        result.totalHeatTransfer += std::abs(model.state.temperature - before);
        result.maxTemperature = std::max(result.maxTemperature, model.state.temperature);
    }

    static std::optional<Scalar> parameter(
        const EquipmentModel& model,
        std::string_view name) noexcept
    {
        for (const auto& item : model.parameters)
            if (item.name == name)
                return item.value;
        return std::nullopt;
    }
};

}
