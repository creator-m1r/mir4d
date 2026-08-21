#pragma once

#include "MirEngine/BRep/Geometry/BRepGeometryLinks.hpp"
#include "MirEngine/BRep/Geometry/BRepGeometryStore.hpp"
#include "MirEngine/BRep/Topology/BRepTopologyStore.hpp"
#include "MirEngine/BRep/Converters/SketchCurveToBRepCurve.hpp"
#include "MirEngine/BRep/Converters/SketchExtrudeSurfaceGeometryBuilder.hpp"
#include "MirEngine/Features/SketchExtrudeBoundary.hpp"
#include "MirEngine/Sketch/SketchProfileGeometryResolver.hpp"

#include <optional>
#include <vector>

namespace mir
{

struct BRepExtrudeBindingResult
{
    bool valid{false};
    std::size_t boundEdges{0};
    std::size_t boundFaces{0};
};

class BRepExtrudeTopologyGeometryBinder
{
public:
    [[nodiscard]] static std::optional<BRepExtrudeBindingResult> bind(
        const SketchGeometryStore& sketchStore,
        const SketchExtrudeBoundary& boundary,
        const BRepTopologyStore& topology,
        BRepGeometryStore& geometry,
        BRepGeometryLinks& links,
        const Vector3& direction,
        Scalar distance) noexcept
    {
        if (!boundary.isClosed() || !(distance > 0.0) || !direction.isFinite())
            return std::nullopt;

        BRepExtrudeBindingResult result;
        std::size_t faceIndex = 0;

        for (const auto& boundaryFace : boundary.faces)
        {
            if (faceIndex >= topology.faces().size())
                return std::nullopt;

            const BRepFaceHandle face = topology.faces()[faceIndex++].self;

            if (boundaryFace.kind == SketchExtrudeBoundaryFaceKind::Bottom ||
                boundaryFace.kind == SketchExtrudeBoundaryFaceKind::Top)
            {
                const BRepPlaneSurface plane{
                    {0.0, 0.0,
                     boundaryFace.kind == SketchExtrudeBoundaryFaceKind::Bottom
                         ? 0.0
                         : distance},
                    {0.0, 0.0, 1.0},
                    {1.0, 0.0, 0.0}};

                BRepSurfaceGeometry surfaceGeometry;
                surfaceGeometry.type = BRepSurfaceType::Plane;
                surfaceGeometry.plane = plane;
                surfaceGeometry.uRange = BRepRange{-1.0, 1.0};
                surfaceGeometry.vRange = BRepRange{-1.0, 1.0};

                const auto surface = geometry.addSurface(surfaceGeometry);
                links.linkFace(face, surface);
                ++result.boundFaces;
                continue;
            }

            if (boundaryFace.edges.empty())
                return std::nullopt;

            const auto curve = SketchProfileGeometryResolver::resolve(
                sketchStore,
                boundaryFace.sourceGeometryID);
            if (!curve || curve->construction)
                return std::nullopt;

            const auto surface = SketchExtrudeSurfaceGeometryBuilder::addSideSurface(
                *curve,
                direction,
                distance,
                geometry);
            if (!surface)
                return std::nullopt;

            links.linkFace(face, *surface);
            ++result.boundFaces;

            for (const auto& boundaryEdge : boundaryFace.edges)
            {
                const auto converted = SketchCurveToBRepCurve::convert(*curve);
                if (!converted)
                    return std::nullopt;

                const BRepCurveHandle curveHandle = geometry.addCurve(*converted);

                for (const auto& edgeRecord : topology.edges())
                {
                    if (!links.curveForEdge(edgeRecord.self))
                    {
                        links.linkEdge(
                            edgeRecord.self,
                            curveHandle,
                            boundaryEdge.reversed);
                        ++result.boundEdges;
                        break;
                    }
                }
            }
        }

        result.valid = result.boundFaces > 0 && result.boundEdges > 0;
        return result;
    }
};

}
