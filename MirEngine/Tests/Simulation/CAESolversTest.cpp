#include "MirEngine/Simulation/SimulationWorld.hpp"

#include <cassert>
#include <cmath>

int main()
{
    mir::World world;
    auto part = world.create(mir::WorldObjectType::Part, "part");
    part->setMaterial({"steel", "", 0.0, 350.0, 101325.0});

    mir::SimulationWorld sim;
    sim.settings().running = true;
    sim.settings().timeScale = 1.0;
    sim.start();

    auto& state = sim.states().state(part->id());
    state.flowRate = 5.0;
    state.composition["reactant"] = 1.0;

    for (int i = 0; i < 30; ++i)
        sim.step(world, 0.1);

    const auto* s = sim.states().find(part->id());
    assert(s != nullptr);

    assert(std::isfinite(s->temperature));
    assert(std::isfinite(s->density));
    assert(std::isfinite(s->aerodynamicDrag));
    assert(s->density > 0.0);
    assert(s->temperature < 350.0);
    assert(s->velocity.x > 0.0);
    assert(s->aerodynamicDrag > 0.0);
    assert(s->composition.at("product") > 0.0);

    double baselineStress = s->stress;

    mir::World world2;
    auto soft = world2.create(mir::WorldObjectType::Part, "soft");
    soft->setMaterial({"soft", "", 0.0, 350.0, 101325.0, 1.0e11, 1.2e-5, 0.3});

    mir::SimulationWorld sim2;
    sim2.settings().running = true;
    sim2.start();
    auto& state2 = sim2.states().state(soft->id());
    state2.flowRate = 5.0;
    state2.composition["reactant"] = 1.0;

    for (int i = 0; i < 30; ++i)
        sim2.step(world2, 0.1);

    const auto* s2 = sim2.states().find(soft->id());
    assert(s2 != nullptr);
    assert(s2->stress < baselineStress);
    assert(s2->youngModulus == 1.0e11);

    return 0;
}
