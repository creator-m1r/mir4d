#pragma once

#include "MirEngine/BRep/Geometry/BRepGeometryLinks.hpp"
#include "MirEngine/BRep/Geometry/BRepGeometryStore.hpp"
#include "MirEngine/BRep/Converters/SketchExtrudeCurveGeometryBuilder.hpp"
#include "MirEngine/BRep/Converters/SketchExtrudeSurfaceGeometryBuilder.hpp"
#include "MirEngine/Features/SketchExtrudeBoundary.hpp"
#include "MirEngine/Sketch/SketchProfileGeometryResolver.hpp"

#include <optional>

namespace mir
{

struct BRepExtrudeGeometryBuildResult
{
    bool valid{false};
};

class BRepExtrudeGeometryBuilder
{
public:
    [[nodiscard]] static std::optional<BRepExtrudeGeometryBuildResult> build(
        const SketchGeometryStore& sketchStore,
        const SketchExtrudeBoundary& boundary,
        BRepGeometryStore& geometry,
        BRepGeometryLinks& links,
        const Vector3& direction,
        Scalar distance) noexcept
    {
        if (!boundary.isClosed() || !(distance > 0.0) || !direction.isFinite())
            return std::nullopt;

        for (const auto& face : boundary.faces)
        {
            if (face.kind != SketchExtrudeBoundaryFaceKind::Side)
                continue;

            if (face.sourceGeometryID == 0)
                return std::nullopt;

            const auto curve = SketchProfileGeometryResolver::resolve(
                sketchStore,
                face.sourceGeometryID);
            if (!curve || curve->construction)
                return std::nullopt;

            const auto surface = SketchExtrudeSurfaceGeometryBuilder::addSideSurface(
                *curve,
                direction,
                distance,
                geometry);
            if (!surface)
                return std::nullopt;

            // Face-to-surface association is owned by the topology binder.
            (void)links;
        }

        return BRepExtrudeGeometryBuildResult{true};
    }
};

} // namespace mir
