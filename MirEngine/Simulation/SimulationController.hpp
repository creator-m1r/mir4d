#pragma once

#include "SimulationTelemetry.hpp"
#include "SimulationWorld.hpp"

namespace mir
{

class SimulationController
{
public:
    void start() noexcept { world_.start(); }
    void pause() noexcept { world_.pause(); }

    void reset() noexcept
    {
        world_.reset();
        telemetry_ = {};
    }

    void update(World& world, Scalar deltaSeconds) noexcept
    {
        if (deltaSeconds <= 0.0 || !world_.settings().running)
            return;

        world_.step(world, deltaSeconds);
        world.advance(mir4d::Time(world_.settings().time));
        refreshTelemetry(world);
    }

    [[nodiscard]] SimulationWorld& simulation() noexcept { return world_; }
    [[nodiscard]] const SimulationWorld& simulation() const noexcept { return world_; }
    [[nodiscard]] const SimulationTelemetry& telemetry() const noexcept { return telemetry_; }

private:
    void refreshTelemetry(const World& world) noexcept
    {
        (void)world;
        const auto& source = world_.telemetry();
        telemetry_.time = source.time;
        telemetry_.running = source.running;
        telemetry_.objects = source.objects;
        telemetry_.totalFlowRate = source.totalFlowRate;
        telemetry_.averageTemperature = source.averageTemperature;
        telemetry_.averagePressure = source.averagePressure;
        telemetry_.totalDrag = source.totalDrag;
        telemetry_.maxTemperature = source.maxTemperature;
        telemetry_.minTemperature = source.minTemperature;
        telemetry_.maxPressure = source.maxPressure;
        telemetry_.maxStress = source.maxStress;
        telemetry_.maxVelocity = source.maxVelocity;
        telemetry_.maxAcoustic = source.maxAcoustic;
    }

    SimulationWorld world_{};
    SimulationTelemetry telemetry_{};
};

} // namespace mir
