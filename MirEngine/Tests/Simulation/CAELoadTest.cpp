#include "MirEngine/Simulation/SimulationWorld.hpp"

#include <cassert>

int main()
{
    {
        mir::World world;
        auto part = world.create(mir::WorldObjectType::Part, "part");
        part->setMaterial({"steel", "", 0.0, 293.15, 101325.0});

        mir::SimulationWorld sim;
        sim.settings().running = true;
        sim.start();

        auto& state = sim.states().state(part->id());
        state.heatFlux = 1.0e7;

        for (int i = 0; i < 60; ++i)
            sim.step(world, 0.1);

        const auto* s = sim.states().find(part->id());
        assert(s != nullptr);
        assert(s->temperature > 300.0);
    }

    {
        mir::World world;
        auto part = world.create(mir::WorldObjectType::Part, "part");
        part->setMaterial({"steel", "", 0.0, 293.15, 101325.0});

        mir::SimulationWorld sim;
        sim.settings().running = true;
        sim.start();

        auto& state = sim.states().state(part->id());
        state.pressureLoad = 1.0e7;

        for (int i = 0; i < 30; ++i)
            sim.step(world, 0.1);

        const auto* s = sim.states().find(part->id());
        assert(s != nullptr);
        assert(s->stress >= 1.0e7 - 1.0);
    }

    {
        mir::World world;
        auto part = world.create(mir::WorldObjectType::Part, "part");
        part->setMaterial({"steel", "", 0.0, 350.0, 101325.0});

        mir::SimulationWorld sim;
        sim.settings().running = true;
        sim.start();

        auto& state = sim.states().state(part->id());
        state.fixed = true;
        state.heatFlux = 1.0e7;

        for (int i = 0; i < 30; ++i)
            sim.step(world, 0.1);

        const auto* s = sim.states().find(part->id());
        assert(s != nullptr);
        assert(s->displacement == 0.0);
        assert(s->stress >= 1.0e7 - 1.0);
    }

    return 0;
}
