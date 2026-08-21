#pragma once

#include "../Rendering/WorldRenderBudget.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace mir
{

enum class SimulationDomain : std::uint8_t
{
    Mechanics,
    Fluid,
    Thermal,
    Chemistry,
    Aerodynamics,
    Acoustics,
    Rendering
};

struct SimulationTaskBudget
{
    SimulationDomain domain{SimulationDomain::Mechanics};
    std::size_t operationsPerTick{0};
    std::size_t minimumOperations{1};
    std::size_t priority{1};
    bool enabled{true};
};

struct SimulationTaskDecision
{
    SimulationDomain domain{SimulationDomain::Mechanics};
    std::size_t operations{0};
    bool execute{false};
};

struct WorldSimulationSchedule
{
    std::vector<SimulationTaskDecision> tasks;
    std::size_t totalOperations{0};
};

class WorldSimulationScheduler
{
public:
    WorldSimulationScheduler()
    {
        configureDefaults();
    }

    void configureDefaults() noexcept
    {
        budgets_ = {
            {SimulationDomain::Mechanics, 1024, 128, 10, true},
            {SimulationDomain::Fluid, 1024, 128, 10, true},
            {SimulationDomain::Thermal, 768, 64, 7, true},
            {SimulationDomain::Chemistry, 512, 32, 5, true},
            {SimulationDomain::Aerodynamics, 768, 64, 6, true},
            {SimulationDomain::Acoustics, 256, 16, 3, true},
            {SimulationDomain::Rendering, 4096, 256, 10, true}};
    }

    [[nodiscard]] WorldSimulationSchedule build(
        const WorldRenderBudget& worldBudget) const
    {
        WorldSimulationSchedule schedule{};
        schedule.tasks.reserve(budgets_.size());

        const std::size_t available = std::max<std::size_t>(1, worldBudget.physicsBudget);

        std::size_t weightSum = 0;
        for (const auto& budget : budgets_)
            if (budget.enabled)
                weightSum += budget.priority;

        if (weightSum == 0)
            return schedule;

        std::size_t allocated = 0;
        for (const auto& budget : budgets_)
        {
            if (!budget.enabled)
            {
                schedule.tasks.push_back({budget.domain, 0, false});
                continue;
            }

            const std::size_t weighted =
                (available * budget.priority) / weightSum;
            const std::size_t operations = std::max(
                budget.minimumOperations,
                std::min(budget.operationsPerTick, weighted));

            schedule.tasks.push_back({budget.domain, operations, operations > 0});
            allocated += operations;
        }

        schedule.totalOperations = allocated;
        return schedule;
    }

    void setEnabled(SimulationDomain domain, bool enabled) noexcept
    {
        for (auto& budget : budgets_)
        {
            if (budget.domain == domain)
            {
                budget.enabled = enabled;
                return;
            }
        }
    }

    void setPriority(SimulationDomain domain, std::size_t priority) noexcept
    {
        for (auto& budget : budgets_)
        {
            if (budget.domain == domain)
            {
                budget.priority = std::max<std::size_t>(1, priority);
                return;
            }
        }
    }

    [[nodiscard]] const std::array<SimulationTaskBudget, 7>& budgets() const noexcept
    {
        return budgets_;
    }

private:
    std::array<SimulationTaskBudget, 7> budgets_{};
};

}
