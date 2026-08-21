#pragma once

#include "EquipmentSimulation.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{

struct EquipmentFlowRequest
{
    Scalar requestedFlow{0.0};
    Scalar availablePressure{0.0};
    Scalar resistance{0.0};
};

struct EquipmentFlowResult
{
    Scalar flowRate{0.0};
    Scalar pressureDrop{0.0};
    Scalar transferredPower{0.0};
};

class EquipmentBehavior
{
public:
    [[nodiscard]] EquipmentFlowResult evaluate(
        const EquipmentSimulationState& state,
        const EquipmentFlowRequest& request) const noexcept
    {
        if (!state.running)
            return {};

        const Scalar efficiency = std::clamp(state.efficiency, 0.0, 1.0);
        const Scalar resistance = std::max(0.0, request.resistance);
        const Scalar availablePressure = std::max(0.0, request.availablePressure);
        const Scalar requestedFlow = std::max(0.0, request.requestedFlow);

        Scalar flow = requestedFlow;

        switch (state.kind)
        {
            case EquipmentKind::Pump:
                flow = std::max(0.0, state.outputPower) * 0.01;
                break;

            case EquipmentKind::Pipe:
                flow = requestedFlow / (1.0 + resistance);
                break;

            case EquipmentKind::Tank:
                flow = requestedFlow;
                break;

            case EquipmentKind::HeatExchanger:
                flow = requestedFlow / (1.0 + resistance * 0.5);
                break;

            case EquipmentKind::Separator:
                flow = requestedFlow * efficiency;
                break;

            case EquipmentKind::Fan:
                flow = std::max(0.0, state.outputPower) * 0.02;
                break;

            case EquipmentKind::Motor:
            case EquipmentKind::Generic:
                flow = requestedFlow;
                break;
        }

        const Scalar pressureDrop = std::min(
            availablePressure,
            flow * resistance);

        return {
            flow,
            pressureDrop,
            flow * pressureDrop * efficiency
        };
    }
};

}
