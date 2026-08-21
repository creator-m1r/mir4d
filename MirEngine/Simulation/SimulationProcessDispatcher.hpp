#pragma once

#include "ProcessTypes.hpp"
#include "SimulationClock.hpp"
#include "SimulationState.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

namespace mir
{

struct SimulationStepContext
{
    Scalar time{0.0};
    Scalar deltaTime{0.0};
    Scalar timeScale{1.0};
    SimulationStateStore* states{nullptr};
};

class SimulationProcessDispatcher
{
public:
    using StepFunction = std::function<void(const SimulationStepContext&)>;

    struct Process
    {
        std::string name;
        StepFunction step;
        bool enabled{true};
    };

    bool add(std::string name, StepFunction step)
    {
        if (name.empty() || !step)
            return false;

        processes_.push_back({std::move(name), std::move(step), true});
        return true;
    }

    bool setEnabled(std::size_t index, bool enabled) noexcept
    {
        if (index >= processes_.size())
            return false;
        processes_[index].enabled = enabled;
        return true;
    }

    void clear() noexcept
    {
        processes_.clear();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return processes_.size();
    }

    void step(const SimulationClock& clock, SimulationStateStore& states) const
    {
        const SimulationStepContext context{
            clock.time(),
            clock.deltaTime(),
            clock.timeScale(),
            &states
        };

        for (const auto& process : processes_)
        {
            if (process.enabled && process.step)
                process.step(context);
        }
    }

private:
    std::vector<Process> processes_;
};

}
