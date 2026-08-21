// MirEngine/Tests/BRep/BRepStepExport.cpp
//
// Native (no OpenCASCADE) exact B-Rep STEP round-trip through merge + write:
// two boxes are merged, written to STEP text, read back and tessellated.
// Expects 24 triangles (12 per box).

#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Builders/BRepPrimAPI_MakeBox.hpp"
#include "MirEngine/BRep/Converters/BRepMerge.hpp"
#include "MirEngine/BRep/Tessellator/BRepTessellator.hpp"
#include "MirEngine/IO/Step/BRepStepBridge.hpp"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

int main()
{
    auto makeBox = [](double a, double b, double c) -> std::shared_ptr<mir::BRepModel>
    {
        auto model = std::make_shared<mir::BRepModel>();
        const auto box = mir::BRepPrimAPI_MakeBox::build(*model, a, b, c);
        model->addRootSolid(box.solid);
        return model;
    };

    std::vector<std::shared_ptr<mir::BRepModel>> sources = {
        makeBox(2.0, 3.0, 4.0),
        makeBox(1.0, 1.0, 1.0)
    };

    std::shared_ptr<mir::BRepModel> merged = mir::mergeBRepModels(sources);
    if (merged->rootSolids().size() != 2)
    {
        std::fprintf(
            stderr,
            "FAIL: merged rootSolids=%zu (expected 2)\n",
            merged->rootSolids().size());
        return 1;
    }

    std::string error;
    std::string text;
    if (!mir::io::step::BRepStepBridge::writeToText(text, *merged, error))
    {
        std::fprintf(stderr, "FAIL: write: %s\n", error.c_str());
        return 1;
    }

    std::shared_ptr<mir::BRepModel> back =
        mir::io::step::BRepStepBridge::readFromText(text, error);
    if (!back)
    {
        std::fprintf(stderr, "FAIL: read: %s\n", error.c_str());
        return 1;
    }

    if (back->rootSolids().size() != 2)
    {
        std::fprintf(
            stderr,
            "FAIL: round-trip rootSolids=%zu (expected 2)\n",
            back->rootSolids().size());
        return 1;
    }

    const mir::TriangleMesh3 mesh = mir::BRepTessellator::tessellateModel(*back);
    if (mesh.triangles.size() != 24)
    {
        std::fprintf(
            stderr,
            "FAIL: triangles=%zu (expected 24)\n",
            mesh.triangles.size());
        return 1;
    }

    std::printf("BRep STEP merge+export round-trip OK (solids=2, triangles=24)\n");
    return 0;
}
