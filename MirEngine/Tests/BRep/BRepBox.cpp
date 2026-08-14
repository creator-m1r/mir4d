#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Builders/BRepPrimAPI_MakeBox.hpp"
#include "MirEngine/BRep/Builders/BRepExtrudeBuilder.hpp"
#include "MirEngine/BRep/Builders/BRepBuilderAPI.hpp"
#include "MirEngine/BRep/Boolean/BRepBooleanAPI.hpp"
#include "MirEngine/BRep/Validator/BRepValidator.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace
{

[[nodiscard]] bool near(const mir::Vector3& a, const mir::Vector3& b, mir::Scalar tolerance = 1e-9)
{
    return (a - b).lengthSquared() <= tolerance * tolerance;
}

[[nodiscard]] bool hasVertex(const mir::BRepModel& model, const mir::Vector3& expected)
{
    const auto& vertices = model.topology().vertices();
    for (const auto& vertex : vertices)
    {
        const mir::BRepPointGeometry* point = model.geometry().point(vertex.point);
        if (point && near(point->point, expected))
            return true;
    }
    return false;
}

} // namespace

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

    // Повёрнутый box (buildOriented): базис повёрнут на 45° вокруг Z.
    {
        mir::BRepModel model;
        const mir::Scalar s = std::sqrt(2.0) * 0.5;
        const auto result = mir::BRepPrimAPI_MakeBox::buildOriented(
            model, 2.0, 3.0, 4.0, mir::Vector3{1.0, 1.0, 0.0},
            mir::Vector3{s, s, 0.0},
            mir::Vector3{-s, s, 0.0});

        assert(result.success);
        assert(result.solid.valid());
        assert(model.topology().vertexCount() == 8);
        assert(model.topology().edgeCount() == 12);
        assert(model.topology().wireCount() == 6);
        assert(model.topology().faceCount() == 6);
        assert(model.topology().shellCount() == 1);
        assert(model.topology().solidCount() == 1);
        assert(model.rootSolids().size() == 1);

        // Все 8 вершин в ожидаемых позициях повёрнутого базиса.
        assert(hasVertex(model, {1.0, 1.0, 0.0}));
        assert(hasVertex(model, {1.0 + 2.0 * s, 1.0 + 2.0 * s, 0.0}));
        assert(hasVertex(model, {1.0 - s, 1.0 + 5.0 * s, 0.0}));
        assert(hasVertex(model, {1.0 - 3.0 * s, 1.0 + 3.0 * s, 0.0}));
        assert(hasVertex(model, {1.0, 1.0, 4.0}));
        assert(hasVertex(model, {1.0 + 2.0 * s, 1.0 + 2.0 * s, 4.0}));
        assert(hasVertex(model, {1.0 - s, 1.0 + 5.0 * s, 4.0}));
        assert(hasVertex(model, {1.0 - 3.0 * s, 1.0 + 3.0 * s, 4.0}));

        const mir::BRepValidator validator;
        assert(validator.validate(model).ok());
    }

    // Некорректный базис отклоняется честно.
    {
        mir::BRepModel model;
        const auto bad = mir::BRepPrimAPI_MakeBox::buildOriented(
            model, 1.0, 1.0, 1.0, mir::Vector3::zero(),
            mir::Vector3{1.0, 0.0, 0.0},
            mir::Vector3{1.0, 0.0, 0.0});
        assert(!bad.success);
        assert(!bad.solid.valid());
        assert(model.topology().vertexCount() == 0);
    }

    // Повёрнутый box честно возвращает NotImplemented в булевых операциях
    // (обобщённый kernel пересечений ещё не реализован).
    {
        mir::BRepModel argument;
        const auto boxA = mir::BRepPrimAPI_MakeBox::build(
            argument, 2.0, 2.0, 2.0, mir::Vector3::zero());
        assert(boxA.success);

        mir::BRepModel tool;
        const mir::Scalar s = std::sqrt(2.0) * 0.5;
        const auto boxB = mir::BRepPrimAPI_MakeBox::buildOriented(
            tool, 2.0, 2.0, 2.0, mir::Vector3{1.0, 1.0, 1.0},
            mir::Vector3{s, s, 0.0},
            mir::Vector3{-s, s, 0.0});
        assert(boxB.success);

        mir::BRepModel fused;
        const auto fuseResult = mir::BRepBooleanAPI::fuse(
            fused, argument, boxA.solid, tool, boxB.solid);
        assert(!fuseResult.success());
        assert(fuseResult.status == mir::BRepBooleanStatus::NotImplemented);

        mir::BRepModel cutOut;
        const auto cutResult = mir::BRepBooleanAPI::cut(
            cutOut, argument, boxA.solid, tool, boxB.solid);
        assert(!cutResult.success());
        assert(cutResult.status == mir::BRepBooleanStatus::NotImplemented);

        mir::BRepModel commonOut;
        const auto commonResult = mir::BRepBooleanAPI::common(
            commonOut, argument, boxA.solid, tool, boxB.solid);
        assert(!commonResult.success());
        assert(commonResult.status == mir::BRepBooleanStatus::NotImplemented);
    }

    return 0;
}
