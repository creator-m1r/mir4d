#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Builders/BRepPrimAPI_MakeBox.hpp"
#include "MirEngine/BRep/Builders/BRepExtrudeBuilder.hpp"
#include "MirEngine/BRep/Builders/BRepBuilderAPI.hpp"
#include "MirEngine/BRep/Validator/BRepValidator.hpp"

#include <cassert>
#include <iostream>

int main()
{
    {
        mir::BRepModel model;
        const auto result = mir::BRepPrimAPI_MakeBox::build(
            model, 100.0, 60.0, 40.0, mir::Vector3::zero());

        assert(result.success);
        assert(result.solid.valid());
        assert(model.topology().vertexCount() == 8);
        assert(model.topology().edgeCount() == 12);
        assert(model.topology().wireCount() == 6);
        assert(model.topology().faceCount() == 6);
        assert(model.topology().shellCount() == 1);
        assert(model.topology().solidCount() == 1);
        assert(model.rootSolids().size() == 1);

        const mir::BRepValidator validator;
        assert(validator.validate(model).ok());
    }

    {
        mir::BRepModel model;
        mir::BRepBuilderAPI builder(model);

        const auto v0 = builder.makeVertex({0.0, 0.0, 0.0});
        const auto v1 = builder.makeVertex({100.0, 0.0, 0.0});
        const auto v2 = builder.makeVertex({100.0, 60.0, 0.0});
        const auto v3 = builder.makeVertex({0.0, 60.0, 0.0});
        assert(v0.valid() && v1.valid() && v2.valid() && v3.valid());

        const auto e0 = builder.makeEdgeLine(v0, v1);
        const auto e1 = builder.makeEdgeLine(v1, v2);
        const auto e2 = builder.makeEdgeLine(v2, v3);
        const auto e3 = builder.makeEdgeLine(v3, v0);
        assert(e0.valid() && e1.valid() && e2.valid() && e3.valid());

        const auto wire = builder.makeWireFromEdges({e0, e1, e2, e3}, true);
        assert(wire.valid());

        const auto extruded = mir::BRepExtrudeBuilder::extrudeWire(
            model, wire, mir::Vector3{0.0, 0.0, 1.0}, 40.0);
        assert(extruded.success);
        assert(extruded.solid.valid());
        assert(model.topology().solidCount() >= 1);

        const auto report = mir::BRepValidator{}.validate(model);
        if (!report.ok())
            for (const auto& issue : report.issues) std::cerr << issue.message << '\n';
        assert(report.ok());
    }

    return 0;
}
