#pragma once

#include "MirEngine/BRep/Geometry/BRepGeometry.hpp"
#include "MirEngine/BRep/Topology/BRepTopology.hpp"
#include "MirEngine/BRep/Core/BRepHandles.hpp"

#include <optional>
#include <vector>

namespace mir
{

// Geometry links use canonical typed handles exclusively.
// No parallel ID typedefs are permitted in the BRep module.
struct BRepOrientedCurve
{
    BRepCurveHandle curve{};
    bool reversed{false};
};

struct BRepEdgeGeometryLink
{
    BRepEdgeHandle edge{};
    BRepOrientedCurve curve{};
};

struct BRepFaceGeometryLink
{
    BRepFaceHandle face{};
    BRepSurfaceHandle surface{};
};

class BRepGeometryLinks
{
public:
    void linkEdge(
        BRepEdgeHandle edge,
        BRepCurveHandle curve,
        bool reversed = false)
    {
        if (!edge.valid() || !curve.valid())
            return;

        edgeLinks_.push_back({edge, {curve, reversed}});
    }

    void linkFace(
        BRepFaceHandle face,
        BRepSurfaceHandle surface)
    {
        if (!face.valid() || !surface.valid())
            return;

        faceLinks_.push_back({face, surface});
    }

    [[nodiscard]] std::optional<BRepOrientedCurve> curveForEdge(
        BRepEdgeHandle edge) const noexcept
    {
        for (const auto& link : edgeLinks_)
        {
            if (link.edge == edge)
                return link.curve;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<BRepSurfaceHandle> surfaceForFace(
        BRepFaceHandle face) const noexcept
    {
        for (const auto& link : faceLinks_)
        {
            if (link.face == face)
                return link.surface;
        }
        return std::nullopt;
    }

    [[nodiscard]] const std::vector<BRepEdgeGeometryLink>& edgeLinks() const noexcept
    {
        return edgeLinks_;
    }

    [[nodiscard]] const std::vector<BRepFaceGeometryLink>& faceLinks() const noexcept
    {
        return faceLinks_;
    }

private:
    std::vector<BRepEdgeGeometryLink> edgeLinks_;
    std::vector<BRepFaceGeometryLink> faceLinks_;
};

} // namespace mir
