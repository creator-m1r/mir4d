#pragma once

#include "EquipmentSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string_view>

namespace mir
{

struct FluidFlowResult
{
    std::size_t elementsProcessed{0};
    Scalar totalFlowRate{0.0};
    Scalar maxPressure{0.0};
};

class FluidFlowSolver
{
public:
    [[nodiscard]] FluidFlowResult step(
        EquipmentModelStore& equipment,
        const EquipmentConnectionStore& connections,
        const EquipmentSystem& system,
        Scalar deltaTime) const noexcept
    {
        FluidFlowResult result{};
        const Scalar dt = std::max(0.0, deltaTime);
        if (!system.enabled || !system.running || dt <= 0.0)
            return result;

        for (const auto equipmentId : system.equipmentIds)
        {
            auto* model = equipment.get(equipmentId);
            if (!model || !model->enabled)
                continue;

            const auto type = std::string_view(model->type);
            if (type == "Pump")
                updatePump(*model, dt);
            else if (type == "Valve")
                updateValve(*model);
            else if (type == "Pipe")
                updatePipe(*model, dt);
            else if (type == "Tank")
                updateTank(*model, dt);

            result.totalFlowRate += std::max(0.0, model->state.flowRate);
            result.maxPressure = std::max(result.maxPressure, model->state.pressure);
            ++result.elementsProcessed;
        }

        for (const auto equipmentId : system.equipmentIds)
        {
            for (const auto& connection : connections.forEquipment(equipmentId))
            {
                if (!connection.enabled || connection.process != EquipmentConnectionProcess::Fluid)
                    continue;

                auto* source = equipment.get(connection.sourceEquipmentId);
                auto* target = equipment.get(connection.targetEquipmentId);
                if (!source || !target || !source->enabled || !target->enabled)
                    continue;

                target->state.flowRate = source->state.flowRate;
                target->state.pressure = source->state.pressure;
                target->state.density = source->state.density;
                target->state.viscosity = source->state.viscosity;
                target->state.temperature = source->state.temperature;
                target->state.velocity = source->state.velocity;
            }
        }

        return result;
    }

private:
    static void updatePump(EquipmentModel& model, Scalar dt) noexcept
    {
        const Scalar speed = parameter(model, "speed").value_or(0.0);
        const Scalar pressure = parameter(model, "pressure").value_or(0.0);
        const Scalar targetFlow = parameter(model, "flow").value_or(0.0);

        if (!model.state.running)
        {
            model.state.flowRate *= std::clamp(1.0 - dt * 2.0, 0.0, 1.0);
            return;
        }

        const Scalar speedFactor = std::clamp(speed / 1450.0, 0.0, 4.0);
        const Scalar targetPressure = std::max(0.0, pressure) * speedFactor * speedFactor;
        const Scalar response = std::clamp(dt * 4.0, 0.0, 1.0);

        model.state.pressure += (targetPressure - model.state.pressure) * response;
        model.state.flowRate +=
            (std::max(0.0, targetFlow) * speedFactor - model.state.flowRate) * response;
        model.state.velocity.x = model.state.flowRate;
    }

    static void updateValve(EquipmentModel& model) noexcept
    {
        const Scalar opening = std::clamp(
            parameter(model, "opening").value_or(1.0), 0.0, 1.0);
        model.state.flowRate *= opening;
        model.state.pressure *= 0.5 + 0.5 * opening;
    }

    static void updatePipe(EquipmentModel& model, Scalar dt) noexcept
    {
        const Scalar diameter = std::max(
            parameter(model, "diameter").value_or(0.05), 1.0e-4);
        const Scalar area = 3.14159265358979323846 * diameter * diameter * 0.25;

        if (area > 0.0)
            model.state.velocity.x = model.state.flowRate / area;

        const Scalar resistance = std::clamp(dt * 0.02, 0.0, 1.0);
        model.state.pressure *= (1.0 - resistance);
    }

    static void updateTank(EquipmentModel& model, Scalar dt) noexcept
    {
        const Scalar volume = std::max(
            parameter(model, "volume").value_or(1.0), 1.0e-6);
        Scalar level = std::clamp(
            parameter(model, "level").value_or(0.0), 0.0, volume);

        level = std::clamp(
            level + model.state.flowRate * dt,
            0.0,
            volume);

        setParameter(model, "level", level);
        model.state.flowRate = std::max(0.0, model.state.flowRate);
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

    static bool setParameter(
        EquipmentModel& model,
        std::string_view name,
        Scalar value) noexcept
    {
        for (auto& item : model.parameters)
        {
            if (item.name == name)
            {
                item.value = value;
                return true;
            }
        }
        return false;
    }
};

}
