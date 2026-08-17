#include "MirEngine/Simulation/CAETest.hpp"

#include <cassert>

int main()
{
    mir::WorldMaterialDescriptor material{};
    material.temperature = 350.0;
    material.density = 0.0;

    mir::SimulationState initial{};
    initial.flowRate = 5.0;
    initial.composition["reactant"] = 1.0;

    mir::CAETest lenient;
    lenient.setMaterial(material);
    lenient.setInitialState(initial);
    lenient.setDuration(5.0);
    lenient.setTimeStep(0.1);
    lenient.addCriterion(mir::CAEMetric::Temperature, 0.0, 400.0);
    lenient.addCriterion(mir::CAEMetric::Density, 500.0, 2000.0);
    lenient.addCriterion(mir::CAEMetric::Velocity, 0.0, 1.0);
    lenient.addCriterion(mir::CAEMetric::Composition, 0.0, 1.0, "product");
    lenient.addCriterion(mir::CAEMetric::Stress, 0.0, 1.0e9);

    const bool lenientPassed = lenient.run();
    const auto& lenientResult = lenient.result();
    assert(lenientPassed);
    assert(lenientResult.passed);
    assert(lenientResult.failures.empty());
    assert(lenientResult.maxTemperature <= 400.0);
    assert(lenientResult.maxConcentration > 0.0);
    assert(lenientResult.maxStress > 0.0);

    mir::CAETest strict;
    strict.setMaterial(material);
    strict.setInitialState(initial);
    strict.setDuration(5.0);
    strict.setTimeStep(0.1);
    strict.addCriterion(mir::CAEMetric::Temperature, 0.0, 300.0);

    const bool strictPassed = strict.run();
    const auto& strictResult = strict.result();
    assert(!strictPassed);
    assert(!strictResult.passed);
    assert(!strictResult.failures.empty());

    return 0;
}
