#pragma once

#include "SimulationSolver.hpp"

#include <algorithm>
#include <cmath>
#include <string>

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
        if (!enabled_ || context.deltaTime <= 0.0)
            return;

        const Scalar dt = context.deltaTime;
        context.states.forEach([dt](std::uint64_t, SimulationState& s)
        {
            const Scalar temperature = std::max(1.0, s.temperature);
            const Scalar alpha = std::max(0.0, s.thermalExpansion);
            const Scalar rho0 = std::max(0.01, s.referenceDensity);

            const Scalar density = std::clamp(
                rho0 / (1.0 + alpha * (temperature - 293.15)), 0.01, 2000.0);
            s.density = density;

            const Scalar speed = s.flowRate / std::max(0.01, density);
            s.velocity = {speed, 0.0, 0.0};

            const Scalar dynamicPressure = 0.5 * density * speed * speed;
            s.pressure += (101325.0 + dynamicPressure - s.pressure)
                          * std::clamp(dt * 0.1, 0.0, 1.0);

            s.viscosity = std::max(1e-6, s.viscosity);
            s.deltaTime = dt;
        });
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
        if (!enabled_ || context.deltaTime <= 0.0)
            return;

        const Scalar dt = context.deltaTime;
        const Scalar ambient = 293.15;
        const Scalar refCp = 4181.0;
        context.states.forEach([dt, ambient, refCp](std::uint64_t, SimulationState& s)
        {
            const Scalar cpScale = refCp / std::max(1.0, s.specificHeat);
            const Scalar speed2 = s.velocity.x * s.velocity.x
                                + s.velocity.y * s.velocity.y
                                + s.velocity.z * s.velocity.z;
            const Scalar frictionHeating = 1e-5 * speed2 / std::max(0.01, s.density) * cpScale;

            const Scalar cooling = std::clamp(dt * 0.05 * cpScale, 0.0, 1.0);
            s.temperature += (frictionHeating + (ambient - s.temperature) * cooling) * dt;
            s.temperature += s.heatFlux * dt / (std::max(0.01, s.density) * std::max(1.0, s.specificHeat));
            s.temperature = std::clamp(s.temperature, 1.0, 5000.0);
            s.deltaTime = dt;
        });
    }

private:
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
        if (!enabled_ || context.deltaTime <= 0.0)
            return;

        const Scalar dt = context.deltaTime;
        context.states.forEach([dt](std::uint64_t, SimulationState& s)
        {
            const Scalar temperature = std::max(1.0, s.temperature);
            const Scalar activation = std::clamp((temperature - 300.0) / 100.0, 0.0, 1.0);
            const Scalar rate = 0.05 * activation * dt;

            const Scalar reactant = s.composition["reactant"];
            const Scalar consumed = std::min(reactant, rate);
            s.composition["reactant"] = reactant - consumed;
            s.composition["product"] = s.composition["product"] + consumed;

            s.deltaTime = dt;
        });
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
        if (!enabled_ || context.deltaTime <= 0.0)
            return;

        const Scalar dt = context.deltaTime;
        context.states.forEach([dt](std::uint64_t, SimulationState& s)
        {
            const Scalar speed2 = s.velocity.x * s.velocity.x
                                + s.velocity.y * s.velocity.y
                                + s.velocity.z * s.velocity.z;
            const Scalar density = std::max(0.01, s.density);

            const Scalar dynamicPressure = 0.5 * density * speed2;
            s.aerodynamicPressure = dynamicPressure;
            s.aerodynamicDrag = dynamicPressure * 0.47;
            s.deltaTime = dt;
        });
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
        if (!enabled_ || context.deltaTime <= 0.0)
            return;

        const Scalar dt = context.deltaTime;
        context.states.forEach([dt](std::uint64_t, SimulationState& s)
        {
            const Scalar speed = std::sqrt(
                s.velocity.x * s.velocity.x
                + s.velocity.y * s.velocity.y
                + s.velocity.z * s.velocity.z);
            const Scalar turbulence = speed * speed * speed * 1e-4;
            const Scalar pressureTerm = std::abs(s.pressure - 101325.0) * 1e-5;
            const Scalar level = 10.0 * std::log10(1.0 + turbulence + pressureTerm);
            s.acousticLevel = std::isfinite(level) ? level : 0.0;
            s.deltaTime = dt;
        });
    }

private:
    bool enabled_{true};
};

class MechanicsSolver final : public ISimulationSolver
{
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "Mechanics"; }
    [[nodiscard]] bool enabled() const noexcept override { return enabled_; }
    void setEnabled(bool value) noexcept override { enabled_ = value; }

    void setCharacteristicLength(Scalar value) noexcept { length_ = std::max(1e-6, value); }

    void solve(SimulationSolveContext& context) noexcept override
    {
        if (!enabled_ || context.deltaTime <= 0.0)
            return;

        const Scalar dt = context.deltaTime;
        const Scalar ambient = 293.15;
        context.states.forEach([dt, ambient, this](std::uint64_t, SimulationState& s)
        {
            const Scalar young = std::max(1.0, s.youngModulus);
            const Scalar alpha = std::max(0.0, s.thermalExpansion);
            const Scalar thermalStress = young * alpha * (s.temperature - ambient);
            const Scalar pressureStress = s.pressure * pressureCoupling_;
            const Scalar totalStress = thermalStress + pressureStress + s.pressureLoad;

            s.stress = totalStress;
            s.strain = std::clamp(totalStress / young, -0.1, 0.1);
            if (s.fixed)
                s.displacement = 0.0;
            else
                s.displacement += s.strain * length_ * dt;
            s.deltaTime = dt;
        });
    }

private:
    bool enabled_{true};
    Scalar length_{1.0};
    Scalar pressureCoupling_{0.01};
};

} // namespace mir
