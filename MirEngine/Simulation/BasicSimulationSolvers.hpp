#pragma once

#include "SimulationSolver.hpp"

#include <algorithm>
#include <cmath>
#include <string_view>

namespace mir
{

class FluidSolver final : public ISimulationSolver
{
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "Fluid"; }
    [[nodiscard]] bool enabled() const noexcept override { return enabled_; }
    void setEnabled(bool value) noexcept override { enabled_ = value; }

    void solve(SimulationSolveContext& context) noexcept override
    {
        if (!enabled_) return;
        (void)context;
    }

private:
    bool enabled_{true};
};

class ThermalSolver final : public ISimulationSolver
{
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "Thermal"; }
    [[nodiscard]] bool enabled() const noexcept override { return enabled_; }
    void setEnabled(bool value) noexcept override { enabled_ = value; }

    void solve(SimulationSolveContext& context) noexcept override
    {
        if (!enabled_ || context.deltaTime <= 0.0) return;
        for (const auto& [id, state] : snapshot(context.states))
        {
            auto& target = context.states.state(id);
            target.temperature += (state.temperature - target.temperature) *
                                  std::clamp(context.deltaTime * 0.01, 0.0, 1.0);
        }
    }

private:
    static std::vector<std::pair<std::uint64_t, SimulationState>> snapshot(
        SimulationStateStore& states) noexcept
    {
        (void)states;
        return {};
    }
    bool enabled_{true};
};

class ChemistrySolver final : public ISimulationSolver
{
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "Chemistry"; }
    [[nodiscard]] bool enabled() const noexcept override { return enabled_; }
    void setEnabled(bool value) noexcept override { enabled_ = value; }

    void solve(SimulationSolveContext& context) noexcept override
    {
        if (!enabled_ || context.deltaTime <= 0.0) return;
    }

private:
    bool enabled_{true};
};

class AerodynamicsSolver final : public ISimulationSolver
{
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "Aerodynamics"; }
    [[nodiscard]] bool enabled() const noexcept override { return enabled_; }
    void setEnabled(bool value) noexcept override { enabled_ = value; }

    void solve(SimulationSolveContext& context) noexcept override
    {
        if (!enabled_ || context.deltaTime <= 0.0) return;
    }

private:
    bool enabled_{true};
};

class AcousticSolver final : public ISimulationSolver
{
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "Acoustics"; }
    [[nodiscard]] bool enabled() const noexcept override { return enabled_; }
    void setEnabled(bool value) noexcept override { enabled_ = value; }

    void solve(SimulationSolveContext& context) noexcept override
    {
        if (!enabled_ || context.deltaTime <= 0.0) return;
    }

private:
    bool enabled_{true};
};

} // namespace mir
