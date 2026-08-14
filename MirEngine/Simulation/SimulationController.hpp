#pragma once

#include "SimulationTelemetry.hpp"
#include "SimulationWorld.hpp"

namespace mir
{

class SimulationController
{
public:
    void start() noexcept { world_.settings().running = true; }
    void pause() noexcept { world_.settings().running = false; }

    void reset() noexcept
    {
        world_.settings().running = false;
        world_.settings().time = 0.0;
        telemetry_ = {};
    }

    void update(World& world, Scalar deltaSeconds) noexcept
    {
        if (deltaSeconds <= 0.0 || !world_.settings().running)
            return;

        world_.step(world, deltaSeconds);
        world.advance(Time(world_.settings().time));
        refreshTelemetry(world);
    }

    [[nodiscard]] SimulationWorld& simulation() noexcept { return world_; }
    [[nodiscard]] const SimulationWorld& simulation() const noexcept { return world_; }
    [[nodiscard]] const SimulationTelemetry& telemetry() const noexcept { return telemetry_; }

private:
    void refreshTelemetry(const World& world) noexcept
    {
        telemetry_.time = world_.settings().time;
        telemetry_.running = world_.settings().running;
        telemetry_.objects = world.size();
    }

    SimulationWorld world_{};
    SimulationTelemetry telemetry_{};
};

} // namespace mir
