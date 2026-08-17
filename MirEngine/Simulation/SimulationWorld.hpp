#pragma once

#include "SimulationTypes.hpp"
#include "SimulationTelemetry.hpp"
#include "SimulationClock.hpp"
#include "SimulationScheduler.hpp"
#include "SimulationSolver.hpp"
#include "BasicSimulationSolvers.hpp"
#include "MaterialLibrary.hpp"
#include "MaterialRegion.hpp"
#include "MaterialInterface.hpp"
#include "MaterialInteractionGraph.hpp"
#include "../World/World.hpp"

#include <cstdint>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace mir
{

class SimulationWorld
{
public:
    SimulationWorld()
    {
        configureDefaultSolvers();
    }

    void setSettings(const SimulationSettings& settings) noexcept
    {
        settings_ = settings;
        clock_.setTime(settings_.time);
        clock_.setTimeScale(settings_.timeScale);
        if (settings_.running)
            clock_.start();
        else
            clock_.pause();
    }

    [[nodiscard]] SimulationSettings& settings() noexcept { return settings_; }
    [[nodiscard]] const SimulationSettings& settings() const noexcept { return settings_; }

    [[nodiscard]] SimulationClock& clock() noexcept { return clock_; }
    [[nodiscard]] const SimulationClock& clock() const noexcept { return clock_; }

    [[nodiscard]] SimulationScheduler& scheduler() noexcept { return scheduler_; }
    [[nodiscard]] const SimulationScheduler& scheduler() const noexcept { return scheduler_; }

    [[nodiscard]] MaterialLibrary& materials() noexcept { return materials_; }
    [[nodiscard]] const MaterialLibrary& materials() const noexcept { return materials_; }

    [[nodiscard]] MaterialRegionStore& materialRegions() noexcept { return materialRegions_; }
    [[nodiscard]] const MaterialRegionStore& materialRegions() const noexcept { return materialRegions_; }

    [[nodiscard]] MaterialInterfaceStore& materialInterfaces() noexcept { return materialInterfaces_; }
    [[nodiscard]] const MaterialInterfaceStore& materialInterfaces() const noexcept { return materialInterfaces_; }

    [[nodiscard]] MaterialInteractionGraph& materialInteractions() noexcept { return materialInteractions_; }
    [[nodiscard]] const MaterialInteractionGraph& materialInteractions() const noexcept { return materialInteractions_; }

    [[nodiscard]] const SimulationStateStore& states() const noexcept { return states_; }
    [[nodiscard]] SimulationStateStore& states() noexcept { return states_; }

    [[nodiscard]] const SimulationTelemetry& telemetry() const noexcept { return telemetry_; }

    void rebuildMaterialInteractions()
    {
        materialInteractions_.rebuild(materialRegions_, materialInterfaces_);
    }

    void setFluid(WorldObject::Id id, const FluidState& state = {}) { fluid_[id] = state; }
    void setThermal(WorldObject::Id id, const ThermalState& state = {}) { thermal_[id] = state; }
    void setChemical(WorldObject::Id id, const ChemicalState& state = {}) { chemical_[id] = state; }
    void setAerodynamic(WorldObject::Id id, const AerodynamicState& state = {}) { aerodynamics_[id] = state; }

    [[nodiscard]] FluidState* fluid(WorldObject::Id id) noexcept { return find(fluid_, id); }
    [[nodiscard]] ThermalState* thermal(WorldObject::Id id) noexcept { return find(thermal_, id); }
    [[nodiscard]] ChemicalState* chemical(WorldObject::Id id) noexcept { return find(chemical_, id); }
    [[nodiscard]] AerodynamicState* aerodynamic(WorldObject::Id id) noexcept { return find(aerodynamics_, id); }

    void start() noexcept
    {
        settings_.running = true;
        clock_.setTimeScale(settings_.timeScale);
        clock_.start();
    }

    void pause() noexcept
    {
        settings_.running = false;
        clock_.pause();
    }

    void reset() noexcept
    {
        settings_.running = false;
        settings_.time = 0.0;
        clock_.reset();
    }

    void step(World& world, Scalar deltaSeconds) noexcept
    {
        if (!settings_.running || deltaSeconds <= 0.0)
            return;

        clock_.setTimeScale(settings_.timeScale);
        clock_.tick(deltaSeconds);
        const Scalar dt = clock_.deltaTime();
        settings_.time = clock_.time();

        if (dt <= 0.0)
            return;

        scheduler_.step(clock_, states_);
        world.advance(mir4d::Time(clock_.time()));

        ensureStates(world);
        SimulationSolveContext solveContext{clock_, states_, dt};
        solvers_.initialize(solveContext);
        solvers_.solve(solveContext);

        refreshTelemetry();

        for (auto& [id, fluid] : fluid_)
        {
            if (settings_.thermal)
            {
                if (const auto it = thermal_.find(id); it != thermal_.end())
                    fluid.temperature += it->second.heatGeneration * dt;
            }

            if (settings_.chemistry)
            {
                if (const auto it = chemical_.find(id); it != chemical_.end())
                    it->second.concentration += it->second.reactionRate * dt;
            }
        }

        for (auto& [id, aero] : aerodynamics_)
        {
            const Scalar speed2 = aero.airVelocity.x * aero.airVelocity.x +
                                  aero.airVelocity.y * aero.airVelocity.y +
                                  aero.airVelocity.z * aero.airVelocity.z;
            aero.drag = 0.5 * fluidDensity(id) * speed2;
        }
    }

private:
    template <typename T>
    static T* find(std::unordered_map<WorldObject::Id, T>& map, WorldObject::Id id) noexcept
    {
        const auto it = map.find(id);
        return it == map.end() ? nullptr : &it->second;
    }

    void configureDefaultSolvers() noexcept
    {
        if (solvers_.size() > 0)
            return;
        solvers_.add(fluidSolver_);
        solvers_.add(thermalSolver_);
        solvers_.add(mechanicsSolver_);
        solvers_.add(chemistrySolver_);
        solvers_.add(aerodynamicsSolver_);
        solvers_.add(acousticSolver_);
    }

    void ensureStates(const World& world) noexcept
    {
        world.forEach([this](WorldObject::Id id, const std::shared_ptr<WorldObject>& object)
        {
            if (initialized_.count(id) > 0)
                return;
            SimulationState& state = states_.state(id);
            state.temperature = object->material().temperature;
            state.density = object->material().density > 0.0
                ? object->material().density
                : 1000.0;
            state.youngModulus = object->material().youngModulus;
            state.thermalExpansion = object->material().thermalExpansion;
            state.poissonRatio = object->material().poissonRatio;
            state.specificHeat = object->material().specificHeat;
            state.thermalConductivity = object->material().thermalConductivity;
            state.referenceDensity = object->material().density > 0.0
                ? object->material().density
                : 1000.0;
            initialized_.insert(id);
        });
    }

    void refreshTelemetry() noexcept
    {
        telemetry_.time = clock_.time();
        telemetry_.running = settings_.running;
        telemetry_.objects = initialized_.size();

        if (initialized_.empty())
            return;

        Scalar maxTemperature = std::numeric_limits<Scalar>::lowest();
        Scalar minTemperature = std::numeric_limits<Scalar>::max();
        Scalar maxPressure = std::numeric_limits<Scalar>::lowest();
        Scalar maxStress = std::numeric_limits<Scalar>::lowest();
        Scalar maxVelocity = 0.0;
        Scalar maxAcoustic = 0.0;
        Scalar totalFlow = 0.0;
        Scalar totalDrag = 0.0;
        Scalar sumTemperature = 0.0;
        Scalar sumPressure = 0.0;
        std::size_t count = 0;

        states_.forEach([&](std::uint64_t, const SimulationState& s)
        {
            const Scalar speed = std::sqrt(
                s.velocity.x * s.velocity.x
                + s.velocity.y * s.velocity.y
                + s.velocity.z * s.velocity.z);
            maxTemperature = std::max(maxTemperature, s.temperature);
            minTemperature = std::min(minTemperature, s.temperature);
            maxPressure = std::max(maxPressure, s.pressure);
            maxStress = std::max(maxStress, s.stress);
            maxVelocity = std::max(maxVelocity, speed);
            maxAcoustic = std::max(maxAcoustic, s.acousticLevel);
            totalFlow += s.flowRate;
            totalDrag += s.aerodynamicDrag;
            sumTemperature += s.temperature;
            sumPressure += s.pressure;
            ++count;
        });

        telemetry_.maxTemperature = maxTemperature;
        telemetry_.minTemperature = minTemperature;
        telemetry_.maxPressure = maxPressure;
        telemetry_.maxStress = maxStress;
        telemetry_.maxVelocity = maxVelocity;
        telemetry_.maxAcoustic = maxAcoustic;
        telemetry_.totalFlowRate = totalFlow;
        telemetry_.totalDrag = totalDrag;
        telemetry_.averageTemperature = count > 0 ? sumTemperature / static_cast<Scalar>(count) : 293.15;
        telemetry_.averagePressure = count > 0 ? sumPressure / static_cast<Scalar>(count) : 101325.0;
    }

    [[nodiscard]] Scalar fluidDensity(WorldObject::Id id) const noexcept
    {
        const auto it = fluid_.find(id);
        return it == fluid_.end() ? 1.225 : it->second.density;
    }

    SimulationSettings settings_{};
    SimulationClock clock_{};
    SimulationScheduler scheduler_{};
    SimulationStateStore states_{};

    SimulationSolverStack solvers_{};
    FluidSolver fluidSolver_{};
    ThermalSolver thermalSolver_{};
    MechanicsSolver mechanicsSolver_{};
    ChemistrySolver chemistrySolver_{};
    AerodynamicsSolver aerodynamicsSolver_{};
    AcousticSolver acousticSolver_{};
    std::unordered_set<WorldObject::Id> initialized_{};
    SimulationTelemetry telemetry_{};

    MaterialLibrary materials_{};
    MaterialRegionStore materialRegions_{};
    MaterialInterfaceStore materialInterfaces_{};
    MaterialInteractionGraph materialInteractions_{};

    std::unordered_map<WorldObject::Id, FluidState> fluid_{};
    std::unordered_map<WorldObject::Id, ThermalState> thermal_{};
    std::unordered_map<WorldObject::Id, ChemicalState> chemical_{};
    std::unordered_map<WorldObject::Id, AerodynamicState> aerodynamics_{};
};

} // namespace mir
