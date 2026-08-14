#pragma once

#include "SimulationClock.hpp"
#include "SimulationProcessDispatcher.hpp"
#include "SimulationState.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace mir
{

enum class SimulationPhase : std::uint8_t
{
    Input,
    Mechanics,
    Fluid,
    Thermal,
    Chemistry,
    Aerodynamics,
    Acoustics,
    Output
};

class SimulationScheduler
{
public:
    using StepFunction = SimulationProcessDispatcher::StepFunction;

    struct Task
    {
        SimulationPhase phase{SimulationPhase::Mechanics};
        std::string name;
        StepFunction step;
        bool enabled{true};
    };

    bool add(SimulationPhase phase, std::string name, StepFunction step)
    {
        if (name.empty() || !step)
            return false;

        tasks_.push_back({phase, std::move(name), std::move(step), true});
        return true;
    }

    bool setEnabled(std::size_t index, bool enabled) noexcept
    {
        if (index >= tasks_.size())
            return false;
        tasks_[index].enabled = enabled;
        return true;
    }

    void clear() noexcept
    {
        tasks_.clear();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return tasks_.size();
    }

    void step(const SimulationClock& clock, SimulationStateStore& states) const
    {
        const SimulationStepContext context{
            clock.time(),
            clock.deltaTime(),
            clock.timeScale(),
            &states
        };

        for (const auto phase : orderedPhases())
        {
            for (const auto& task : tasks_)
            {
                if (task.enabled && task.step && task.phase == phase)
                    task.step(context);
            }
        }
    }

private:
    [[nodiscard]] static constexpr std::array<SimulationPhase, 8> orderedPhases() noexcept
    {
        return {
            SimulationPhase::Input,
            SimulationPhase::Mechanics,
            SimulationPhase::Fluid,
            SimulationPhase::Thermal,
            SimulationPhase::Chemistry,
            SimulationPhase::Aerodynamics,
            SimulationPhase::Acoustics,
            SimulationPhase::Output
        };
    }

    std::vector<Task> tasks_;
};

} // namespace mir
