#include "MirEngine/BRep/BRep.hpp"

#include <cassert>

int main()
{
    mir::BRepModel model;
    assert(model.empty());
    assert(model.rootSolids().empty());

    mir::BRepBuilderAPI builder(model);
    const auto v0 = builder.makeVertex({0.0, 0.0, 0.0});
    const auto v1 = builder.makeVertex({1.0, 0.0, 0.0});
    const auto v2 = builder.makeVertex({0.0, 1.0, 0.0});
    const auto e0 = builder.makeEdgeLine(v0, v1);
    const auto e1 = builder.makeEdgeLine(v1, v2);
    const auto e2 = builder.makeEdgeLine(v2, v0);
    const auto wire = builder.makeWireFromEdges({e0, e1, e2}, true);
    assert(wire.valid());
    const auto face = builder.makePlanarFace(
        wire,
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0},
        {1.0, 0.0, 0.0});
    assert(face.valid());
    const auto shell = builder.makeShell({{face, mir::BRepOrientation::Forward}}, true);
    assert(shell.valid());
    const auto solid = builder.makeSolid({shell});
    assert(solid.valid());

    assert(!model.empty());
    assert(model.containsRootSolid(solid));
    assert(model.rootSolids().size() == 1);

    model.addRootSolid(solid);
    model.addRootSolid(solid);
    assert(model.rootSolids().size() == 1);

    const auto* wireRecord = model.topology().wire(wire);
    const auto* faceRecord = model.topology().face(face);
    const auto* shellRecord = model.topology().shell(shell);
    const auto* solidRecord = model.topology().solid(solid);
    assert(wireRecord && wireRecord->ownerFace == face);
    assert(faceRecord && faceRecord->ownerShell == shell);
    assert(shellRecord && shellRecord->ownerSolid == solid);
    assert(solidRecord && solidRecord->shells.size() == 1);
    assert(wireRecord->closed);
    assert(shellRecord->closed);

    mir::BRepValidator validator;
    assert(validator.validate(model).ok());
    assert(model.isValid());

    const std::size_t wiresBeforeFailure = model.topology().wireCount();
    const auto invalidWire = builder.makeWireFromEdges({e0, e2}, true);
    assert(!invalidWire.valid());
    assert(model.topology().wireCount() == wiresBeforeFailure);
    assert(model.topology().solidCount() == 1);
    assert(model.rootSolids().size() == 1);

    const auto checkpoint = model.checkpoint();
    const auto transientVertex = builder.makeVertex({10.0, 10.0, 10.0});
    assert(transientVertex.valid());
    assert(model.topology().vertexCount() > checkpoint.topology.vertexCount);
    model.rollback(checkpoint);
    assert(model.topology().vertexCount() == checkpoint.topology.vertexCount);
    assert(model.geometry().pointCount() == checkpoint.geometry.pointCount);
    assert(model.containsRootSolid(solid));
    assert(model.isValid());

    model.clear();
    assert(model.empty());
    assert(model.rootSolids().empty());
    assert(!model.containsRootSolid(solid));
    assert(model.topology().solidCount() == 0);
    assert(model.geometry().surfaceCount() == 0);

    return 0;
}
