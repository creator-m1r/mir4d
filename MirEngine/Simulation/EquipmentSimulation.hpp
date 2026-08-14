#pragma once

#include "MechanicalSolver.hpp"
#include "ProcessMaterial.hpp"

#include <algorithm>

namespace mir
{

enum class EquipmentKind
{
    Generic,
    Motor,
    Pump,
    Pipe,
    Tank,
    HeatExchanger,
    Separator,
    Fan
};

struct EquipmentSimulationState
{
    EquipmentKind kind{EquipmentKind::Generic};
    bool running{false};
    Scalar efficiency{1.0};
    Scalar inputPower{0.0};
    Scalar outputPower{0.0};
    Scalar flowRate{0.0};
    Scalar pressure{0.0};
    Scalar temperature{293.15};
};

class EquipmentSimulator
{
public:
    void start(EquipmentSimulationState& state) const noexcept
    {
        state.running = true;
    }

    void stop(EquipmentSimulationState& state) const noexcept
    {
        state.running = false;
        state.outputPower = 0.0;
        state.flowRate = 0.0;
    }

    void step(EquipmentSimulationState& state, Scalar deltaSeconds) const noexcept
    {
        if (!state.running || deltaSeconds <= 0.0)
            return;

        state.efficiency = std::clamp(state.efficiency, 0.0, 1.0);
        state.inputPower = std::max(0.0, state.inputPower);
        state.outputPower = state.inputPower * state.efficiency;

        switch (state.kind)
        {
            case EquipmentKind::Motor:
                state.flowRate = 0.0;
                break;

            case EquipmentKind::Pump:
                state.flowRate = state.outputPower * 0.01;
                state.pressure = state.outputPower * 100.0;
                break;

            case EquipmentKind::Pipe:
                state.flowRate = std::max(0.0, state.flowRate);
                break;

            case EquipmentKind::Fan:
                state.flowRate = state.outputPower * 0.02;
                break;

            case EquipmentKind::Tank:
                state.flowRate = std::max(0.0, state.flowRate);
                break;

            case EquipmentKind::HeatExchanger:
                state.temperature += (state.inputPower - state.outputPower) * 0.001 * deltaSeconds;
                break;

            case EquipmentKind::Separator:
            case EquipmentKind::Generic:
                break;
        }

        state.temperature = std::max(0.0, state.temperature);
        state.pressure = std::max(0.0, state.pressure);
    }
};

} // namespace mir
