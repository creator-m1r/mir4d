#pragma once

#include "SimulationTypes.hpp"
#include "SimulationClock.hpp"
#include "SimulationState.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace mir
{

struct SimulationSolveContext
{
    SimulationClock& clock;
    SimulationStateStore& states;
    Scalar deltaTime{0.0};
};

class ISimulationSolver
{
public:
    virtual ~ISimulationSolver() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual bool enabled() const noexcept = 0;
    virtual void setEnabled(bool value) noexcept = 0;

    virtual void initialize(SimulationSolveContext& context) noexcept
    {
        (void)context;
    }

    virtual void solve(SimulationSolveContext& context) noexcept = 0;

    virtual void finalize(SimulationSolveContext& context) noexcept
    {
        (void)context;
    }
};

class SimulationSolverStack
{
public:
    void add(ISimulationSolver& solver)
    {
        solvers_.push_back(&solver);
    }

    void clear() noexcept
    {
        solvers_.clear();
    }

    void initialize(SimulationSolveContext& context) noexcept
    {
        for (auto* solver : solvers_)
            if (solver && solver->enabled())
                solver->initialize(context);
    }

    void solve(SimulationSolveContext& context) noexcept
    {
        for (auto* solver : solvers_)
            if (solver && solver->enabled())
                solver->solve(context);
    }

    void finalize(SimulationSolveContext& context) noexcept
    {
        for (auto* solver : solvers_)
            if (solver && solver->enabled())
                solver->finalize(context);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return solvers_.size();
    }

private:
    std::vector<ISimulationSolver*> solvers_;
};

} // namespace mir
