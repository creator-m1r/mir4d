#include "MirEngine/Simulation/SimulationController.hpp"

#include <cassert>

int main()
{
    mir::World world;
    auto part = world.create(mir::WorldObjectType::Part, "part");
    part->setMaterial({"steel", "", 0.0, 350.0, 101325.0});

    mir::SimulationController controller;
    controller.start();

    auto& state = controller.simulation().states().state(part->id());
    state.flowRate = 5.0;
    state.composition["reactant"] = 1.0;

    for (int i = 0; i < 30; ++i)
        controller.update(world, 0.1);

    const mir::SimulationTelemetry& telemetry = controller.telemetry();

    assert(telemetry.objects == 1);
    assert(telemetry.time > 0.0);
    assert(telemetry.maxStress > 0.0);
    assert(telemetry.maxTemperature <= 350.0 + 1e-6);
    assert(telemetry.minTemperature > 0.0);
    assert(telemetry.maxVelocity > 0.0);
    assert(telemetry.totalDrag > 0.0);
    assert(telemetry.maxAcoustic >= 0.0);
    assert(telemetry.totalFlowRate > 0.0);

    return 0;
}
