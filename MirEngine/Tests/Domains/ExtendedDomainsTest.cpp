#include "MirEngine/Acoustics/AcousticWorld.hpp"
#include "MirEngine/Chemistry/ChemicalCompositionSolver.hpp"
#include "MirEngine/Materials/MaterialStateBinding.hpp"
#include "MirEngine/Mechanics/MechanicsWorld.hpp"
#include "MirEngine/Physics/PhysicsWorld.hpp"
#include "MirEngine/Sketch/SketchInferenceEngine.hpp"

#include <cassert>

int main()
{
    mir::AcousticWorld acoustics;
    mir::ChemicalCompositionSolver chemistry;
    mir::MaterialStateBinding materials;
    mir::MechanicsWorld mechanics;
    mir::PhysicsWorld physics;
    mir::SketchInferenceEngine inference;

    (void)chemistry;
    (void)materials;
    (void)inference;
    (void)physics;

    assert(mechanics.empty());
    assert(acoustics.events().empty());

    return 0;
}
