#pragma once

#include "ProcessMaterial.hpp"
#include "SimulationClock.hpp"
#include "SimulationProcessGraph.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace mir
{

class ProcessSimulator
{
public:
    explicit ProcessSimulator(SimulationProcessGraph& graph) noexcept
        : graph_(graph)
    {
    }

    void reset() noexcept
    {
        materials_.clear();
        lastStep_ = 0.0;
    }

    void registerMaterial(ProcessMaterial material)
    {
        materials_[material.id] = std::move(material);
    }

    [[nodiscard]] ProcessMaterial* material(std::uint64_t id) noexcept
    {
        const auto it = materials_.find(id);
        return it == materials_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const ProcessMaterial* material(std::uint64_t id) const noexcept
    {
        const auto it = materials_.find(id);
        return it == materials_.end() ? nullptr : &it->second;
    }

    void step(const SimulationClock& clock) noexcept
    {
        const Scalar delta = clock.deltaTime();
        if (delta <= 0.0)
        {
            lastStep_ = 0.0;
            return;
        }

        lastStep_ = delta;

        for (const auto& connection : graph_.connections())
        {
            if (!connection.enabled || connection.materialId == 0)
                continue;

            const auto materialIt = materials_.find(connection.materialId);
            if (materialIt == materials_.end())
                continue;

            const auto* source = graph_.node(connection.sourceNode);
            const auto* target = graph_.node(connection.targetNode);
            if (!source || !target || !source->enabled || !target->enabled)
                continue;

            auto& materialState = materialIt->second;
            materialState.mass = std::max(0.0, materialState.mass);
        }
    }

    [[nodiscard]] Scalar lastStep() const noexcept { return lastStep_; }

private:
    SimulationProcessGraph& graph_;
    std::unordered_map<std::uint64_t, ProcessMaterial> materials_;
    Scalar lastStep_{0.0};
};

}
