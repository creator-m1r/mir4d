#pragma once

#include "EquipmentSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace mir
{

struct EquipmentSimulationResult
{
    std::size_t equipmentProcessed{0};
    std::size_t connectionsProcessed{0};
    Scalar deltaTime{0.0};
};

class EquipmentSystemSimulator
{
public:
    [[nodiscard]] EquipmentSimulationResult step(
        EquipmentModelStore& equipment,
        const EquipmentConnectionStore& connections,
        const EquipmentSystem& system,
        Scalar deltaTime) noexcept
    {
        EquipmentSimulationResult result{};
        result.deltaTime = std::max(0.0, deltaTime);

        if (!system.enabled || !system.running || result.deltaTime <= 0.0)
            return result;

        for (const auto equipmentId : system.equipmentIds)
        {
            auto* model = equipment.get(equipmentId);
            if (!model || !model->enabled)
                continue;

            updateEquipment(*model, result.deltaTime);
            ++result.equipmentProcessed;
        }

        for (const auto equipmentId : system.equipmentIds)
        {
            for (const auto& connection : connections.forEquipment(equipmentId))
            {
                if (!connection.enabled)
                    continue;
                if (processedConnections_.contains(connection.id))
                    continue;

                auto* source = equipment.get(connection.sourceEquipmentId);
                auto* target = equipment.get(connection.targetEquipmentId);
                if (!source || !target || !source->enabled || !target->enabled)
                    continue;

                transfer(source->state, target->state, connection.process, result.deltaTime);
                processedConnections_.insert(connection.id);
                ++result.connectionsProcessed;
            }
        }

        processedConnections_.clear();
        return result;
    }

private:
    static void updateEquipment(EquipmentModel& model, Scalar dt) noexcept
    {
        const auto speed = parameter(model, "speed");
        const auto power = parameter(model, "power");
        const auto pressure = parameter(model, "pressure");

        if (speed.has_value())
            model.state.velocity.x = speed.value() * 0.001;

        if (power.has_value())
            model.state.acceleration.x = power.value() * 0.001;

        if (pressure.has_value())
            model.state.pressure = std::max(0.0, pressure.value());

        model.state.time += dt;
        model.state.deltaTime = dt;
    }

    static void transfer(
        SimulationState& source,
        SimulationState& target,
        EquipmentConnectionProcess process,
        Scalar dt) noexcept
    {
        switch (process)
        {
        case EquipmentConnectionProcess::Fluid:
            target.flowRate = source.flowRate;
            target.pressure = source.pressure;
            target.density = source.density;
            target.viscosity = source.viscosity;
            target.temperature = source.temperature;
            target.velocity = source.velocity;
            break;

        case EquipmentConnectionProcess::Mechanical:
            target.velocity = source.velocity;
            target.acceleration = source.acceleration;
            break;

        case EquipmentConnectionProcess::Thermal:
            target.temperature +=
                (source.temperature - target.temperature) *
                std::clamp(dt * 0.25, 0.0, 1.0);
            break;

        case EquipmentConnectionProcess::Electrical:
            target.pressure = source.pressure;
            break;

        case EquipmentConnectionProcess::Control:
            target.running = source.running;
            target.paused = source.paused;
            break;
        }
    }

    static std::optional<Scalar> parameter(
        const EquipmentModel& model,
        std::string_view name) noexcept
    {
        for (const auto& parameter : model.parameters)
        {
            if (parameter.name == name)
                return parameter.value;
        }
        return std::nullopt;
    }

    std::unordered_set<std::uint64_t> processedConnections_;
};

} // namespace mir
