#pragma once

#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Tessellator/BRepTessellator.hpp"
#include "MirEngine/BRep/Validator/BRepValidator.hpp"

#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Geometry/Solid/FacetedSolid.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Math/Point.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace mir
{

using CanonicalModel = Model;
using CanonicalSolid = Solid3;
using CanonicalMesh = TriangleMesh3;
using CanonicalPoint = Point3;

struct BRepToModelOptions
{
    BRepTessellationOptions tessellation{};
    bool validateBeforeConvert{true};
    BRepTolerance tolerance{DefaultBRepTolerance};
};

struct BRepToModelResult
{
    bool success{false};
    std::shared_ptr<CanonicalModel> model{};
    BRepValidationReport report{};
};

class BRepToModel
{
public:
    [[nodiscard]] static BRepToModelResult convertSolid(
        const BRepModel& brep,
        BRepSolidHandle solidHandle,
        BRepToModelOptions options = {})
    {
        BRepToModelResult result{};

        if (!solidHandle.valid() || !brep.topology().solid(solidHandle))
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                solidHandle.index,
                "BRepToModel: solid handle is invalid");
            return result;
        }

        if (options.validateBeforeConvert)
        {
            const BRepValidator validator(options.tolerance);
            result.report = validator.validate(brep);
            if (!result.report.ok())
                return result;
        }

        CanonicalMesh mesh = BRepTessellator::tessellateSolid(
            brep,
            solidHandle,
            options.tessellation);

        if (mesh.vertices.empty() || mesh.triangles.empty() || !mesh.isValid())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                solidHandle.index,
                "BRepToModel: tessellation produced an empty or invalid mesh");
            return result;
        }

        CanonicalSolid solid = meshToSolid(mesh);
        if (!solid.isValid())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                solidHandle.index,
                "BRepToModel: failed to build solid from mesh");
            return result;
        }

        auto model = std::make_shared<CanonicalModel>();
        model->setSolid(std::move(solid));
        model->setMesh(std::move(mesh));

        if (!model->isValid())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                solidHandle.index,
                "BRepToModel: resulting model is invalid");
            return result;
        }

        result.model = std::move(model);
        result.success = true;
        return result;
    }

    [[nodiscard]] static BRepToModelResult convertFirstRoot(
        const BRepModel& brep,
        BRepToModelOptions options = {})
    {
        BRepToModelResult result{};
        if (brep.rootSolids().empty())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                InvalidBRepIndex,
                "BRepToModel: BRep model has no root solids");
            return result;
        }
        return convertSolid(brep, brep.rootSolids().front(), options);
    }

private:
    [[nodiscard]] static CanonicalSolid meshToSolid(const CanonicalMesh& mesh)
    {
        std::vector<CanonicalPoint> vertices;
        vertices.reserve(mesh.vertices.size());
        for (const CanonicalPoint& point : mesh.vertices)
            vertices.push_back(point);

        std::vector<CanonicalSolid::Triangle> triangles;
        triangles.reserve(mesh.triangles.size());
        for (const auto& triangle : mesh.triangles)
            triangles.push_back({triangle.a, triangle.b, triangle.c});

        return CanonicalSolid(std::move(vertices), std::move(triangles));
    }
};

}
