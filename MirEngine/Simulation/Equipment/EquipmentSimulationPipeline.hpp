#pragma once

#include "EquipmentSystemController.hpp"
#include "EquipmentSystemSimulator.hpp"
#include "EquipmentSafetySystem.hpp"
#include "FluidFlowSolver.hpp"
#include "ThermalTransferSolver.hpp"

#include <cstdint>

namespace mir
{

struct EquipmentSimulationStepResult
{
    EquipmentSimulationResult simulation{};
    FluidFlowResult fluid{};
    ThermalTransferResult thermal{};
    std::size_t safetyEvents{0};
    bool running{false};
    bool emergencyStopped{false};
};

class EquipmentSimulationPipeline
{
public:
    [[nodiscard]] EquipmentSimulationStepResult step(
        EquipmentModelStore& equipment,
        const EquipmentConnectionStore& connections,
        EquipmentSystem& system,
        Scalar deltaTime) noexcept
    {
        EquipmentSimulationStepResult result{};

        if (!system.enabled || !system.running)
        {
            result.running = false;
            result.emergencyStopped =
                controller_.state() == EquipmentSystemControlState::EmergencyStopped;
            return result;
        }

        result.simulation = simulator_.step(
            equipment,
            connections,
            system,
            deltaTime);

        result.fluid = fluid_.step(
            equipment,
            connections,
            system,
            deltaTime);

        result.thermal = thermal_.step(
            equipment,
            connections,
            system,
            deltaTime);

        safety_.evaluate(equipment, system);
        result.safetyEvents = safety_.events().size();

        if (safety_.hasCriticalEvent())
        {
            controller_.emergencyStop(equipment, system);
            result.emergencyStopped = true;
            result.running = false;
            return result;
        }

        result.running = system.running;
        result.emergencyStopped =
            controller_.state() == EquipmentSystemControlState::EmergencyStopped;
        return result;
    }

    [[nodiscard]] EquipmentSystemController& controller() noexcept
    {
        return controller_;
    }

    [[nodiscard]] EquipmentSafetySystem& safety() noexcept
    {
        return safety_;
    }

private:
    EquipmentSystemSimulator simulator_{};
    FluidFlowSolver fluid_{};
    ThermalTransferSolver thermal_{};
    EquipmentSystemController controller_{};
    EquipmentSafetySystem safety_{};
};

}
