// MirEngine/Tests/BRep/BRepStep.cpp
// Native B-Rep STEP codec round-trip test (no OpenCASCADE).

#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Builders/BRepPrimAPI_MakeBox.hpp"
#include "MirEngine/BRep/Tessellator/BRepTessellator.hpp"
#include "MirEngine/IO/Step/BRepStepBridge.hpp"

#include <cstdio>
#include <memory>
#include <string>

int main()
{
    mir::BRepModel model;

    const mir::BRepMakeBoxResult box = mir::BRepPrimAPI_MakeBox::build(
        model, 2.0, 3.0, 4.0, mir::Vector3::zero());
    if (!box.success)
    {
        std::printf("FAIL: box build failed\n");
        return 1;
    }
    model.addRootSolid(box.solid);

    std::string stepText;
    std::string error;
    if (!mir::io::step::BRepStepBridge::writeToText(stepText, model, error))
    {
        std::printf("FAIL: writeToText: %s\n", error.c_str());
        return 1;
    }

    if (stepText.find("MANIFOLD_SOLID_BREP") == std::string::npos ||
        stepText.find("FACE_SURFACE") == std::string::npos)
    {
        std::printf("FAIL: written STEP missing B-Rep entities\n");
        return 1;
    }

    std::shared_ptr<mir::BRepModel> loaded =
        mir::io::step::BRepStepBridge::readFromText(stepText, error);
    if (!loaded)
    {
        std::printf("FAIL: readFromText: %s\n", error.c_str());
        return 1;
    }

    if (loaded->rootSolids().empty())
    {
        std::printf("FAIL: loaded model has no root solids\n");
        return 1;
    }

    const mir::BRepSolid* solid = loaded->topology().solid(loaded->rootSolids().front());
    if (!solid || solid->shells.empty())
    {
        std::printf("FAIL: loaded solid has no shells\n");
        return 1;
    }

    const mir::BRepShell* shell = loaded->topology().shell(solid->shells.front());
    if (!shell || shell->faces.size() != 6)
    {
        std::printf("FAIL: expected 6 faces, got %zu\n",
                    shell ? shell->faces.size() : 0);
        return 1;
    }

    mir::BRepTessellationOptions opts;
    opts.deflection = 0.1;
    const mir::TriangleMesh3 mesh =
        mir::BRepTessellator::tessellateModel(*loaded, opts);
    if (mesh.triangles.size() != 12)
    {
        std::printf("FAIL: expected 12 triangles after tessellation, got %zu\n",
                    mesh.triangles.size());
        return 1;
    }

    std::printf("BRep STEP round-trip OK (faces=%zu, triangles=%zu)\n",
                shell->faces.size(), mesh.triangles.size());
    return 0;
}
