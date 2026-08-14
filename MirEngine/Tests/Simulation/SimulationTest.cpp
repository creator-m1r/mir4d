#include "MirEngine/Simulation/ProcessMaterialTransfer.hpp"

#include <cassert>
#include <cmath>

int main()
{
    mir::ProcessGraph graph;
    graph.setState(1, mir::ProcessState{});
    graph.setState(2, mir::ProcessState{});

    auto* source = graph.state(1);
    auto* target = graph.state(2);
    assert(source != nullptr);
    assert(target != nullptr);

    source->flow = 10.0;
    source->pressure = 200.0;
    source->temperature = 350.0;
    graph.connect({1, 2, mir::ProcessPortType::Material, 0.5});
    graph.start();

    mir::ProcessMaterialTransferSystem system;
    mir::SimulationMaterial water;
    water.id = "water";
    water.name = "Water";
    system.registerMaterial(water);
    assert(system.materials().find("water") != nullptr);

    system.step(graph, 1.0);

    assert(target->flow > 0.0);
    assert(target->pressure > 0.0);
    assert(target->temperature > 293.15);
    assert(std::isfinite(target->efficiency));

    return 0;
}
