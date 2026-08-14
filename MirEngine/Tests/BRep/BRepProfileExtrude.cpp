#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Builders/BRepMakeRectangleProfile.hpp"
#include "MirEngine/BRep/Builders/BRepExtrudeBuilder.hpp"
#include "MirEngine/BRep/Tessellator/BRepTessellator.hpp"
#include "MirEngine/BRep/Validator/BRepValidator.hpp"
#include "MirEngine/BRep/Geometry/BRepAdaptor.hpp"

#include <cassert>
#include <iostream>

int main()
{
    {
        mir::BRepModel model;
        const auto profile = mir::BRepMakeRectangleProfile::buildXY(model, 100.0, 60.0);
        assert(profile.success);
        assert(profile.wire.valid());
        const auto extruded = mir::BRepExtrudeBuilder::extrudeWire(model, profile.wire, mir::Vector3{0.0, 0.0, 1.0}, 40.0);
        assert(extruded.success);
        assert(extruded.solid.valid());
        const mir::BRepValidator validator;
        const auto report = validator.validate(model);
        if (!report.ok()) for (const auto& issue : report.issues) std::cerr << issue.message << '\n';
        assert(report.ok());
        const mir::TriangleMesh3 mesh = mir::BRepTessellator::tessellateSolid(model, extruded.solid);
        assert(!mesh.vertices.empty());
        assert(!mesh.triangles.empty());
        assert(mesh.isValid());
    }
    {
        mir::BRepModel model;
        const auto outer = mir::BRepMakeRectangleProfile::buildXY(model, 100.0, 60.0, mir::Vector3{0.0, 0.0, 0.0});
        const auto hole = mir::BRepMakeRectangleProfile::buildXY(model, 20.0, 10.0, mir::Vector3{20.0, 20.0, 0.0});
        assert(outer.success && hole.success);
        const auto extruded = mir::BRepExtrudeBuilder::extrudeWireWithHoles(model, outer.wire, {hole.wire}, mir::Vector3{0.0, 0.0, 1.0}, 15.0);
        assert(extruded.success);
        const mir::TriangleMesh3 mesh = mir::BRepTessellator::tessellateSolid(model, extruded.solid);
        assert(mesh.isValid());
        assert(mesh.triangles.size() > 8);
    }
    {
        mir::BRepModel model;
        const auto profile = mir::BRepMakeRectangleProfile::buildXY(model, 10.0, 5.0);
        assert(profile.success);
        mir::BRepAdaptor_Curve curve(model, profile.edges[0]);
        assert(curve.isBound());
        assert(curve.type() == mir::BRepCurveType::Line);
        const mir::Vector3 a = curve.startPoint();
        const mir::Vector3 b = curve.endPoint();
        assert((b - a).length() > 0.0);
        assert(curve.lengthEstimate(8) > 0.0);
    }
    return 0;
}
