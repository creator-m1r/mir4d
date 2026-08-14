#pragma once

#include "EquipmentSimulation.hpp"
#include "ProcessMaterial.hpp"
#include "SimulationProcessGraph.hpp"

#include <algorithm>
#include <cstdint>
#include <unordered_map>

namespace mir
{

struct ProcessFlowPortState
{
    std::uint64_t portId{0};
    Scalar amount{0.0};
    Scalar flowRate{0.0};
};

class ProcessFlowSimulator
{
public:
    explicit ProcessFlowSimulator(SimulationProcessGraph& graph) noexcept
        : graph_(graph)
    {
    }

    void reset() noexcept
    {
        portStates_.clear();
    }

    void setPortState(std::uint64_t portId, Scalar amount, Scalar flowRate) noexcept
    {
        portStates_[portId] = ProcessFlowPortState{
            portId,
            std::max(0.0, amount),
            std::max(0.0, flowRate)
        };
    }

    [[nodiscard]] const ProcessFlowPortState* portState(std::uint64_t portId) const noexcept
    {
        const auto it = portStates_.find(portId);
        return it == portStates_.end() ? nullptr : &it->second;
    }

    void step(Scalar deltaSeconds) noexcept
    {
        if (deltaSeconds <= 0.0)
            return;

        for (const auto& connection : graph_.connections())
        {
            if (!connection.enabled)
                continue;

            auto sourceIt = portStates_.find(connection.sourcePort);
            auto targetIt = portStates_.find(connection.targetPort);
            if (sourceIt == portStates_.end() || targetIt == portStates_.end())
                continue;

            auto& source = sourceIt->second;
            auto& target = targetIt->second;

            const Scalar transferred = std::min(
                source.amount,
                source.flowRate * deltaSeconds);

            source.amount -= transferred;
            target.amount += transferred;
            target.flowRate = source.flowRate;
        }
    }

private:
    SimulationProcessGraph& graph_;
    std::unordered_map<std::uint64_t, ProcessFlowPortState> portStates_;
};

} // namespace mir
