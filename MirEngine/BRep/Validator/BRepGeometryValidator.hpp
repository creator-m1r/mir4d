#pragma once

// MirEngine/BRep/Validator/BRepGeometryValidator.hpp
// Geometry-link validation for the canonical B-Rep validation layer.
//
// This validator checks that topology-to-geometry links resolve to live
// geometry records. Full structural validation remains the responsibility
// of BRepValidator.

#include "MirEngine/BRep/Geometry/BRepGeometryLinks.hpp"
#include "MirEngine/BRep/Geometry/BRepGeometryStore.hpp"
#include "MirEngine/BRep/Topology/BRepTopologyStore.hpp"

#include <cstddef>

namespace mir
{

struct BRepGeometryValidationResult
{
    bool valid{false};
    std::size_t missingEdgeGeometry{0};
    std::size_t missingFaceGeometry{0};
};

class BRepGeometryValidator
{
public:
    [[nodiscard]] static BRepGeometryValidationResult validate(
        const BRepTopologyStore& topology,
        const BRepGeometryStore& geometry,
        const BRepGeometryLinks& links) noexcept
    {
        BRepGeometryValidationResult result;

        for (const auto& edge : topology.edges())
        {
            const auto curve = links.curveForEdge(edge.self);
            if (!curve || !curve->curve.valid() || !geometry.curve(curve->curve))
                ++result.missingEdgeGeometry;
        }

        for (const auto& face : topology.faces())
        {
            const auto surface = links.surfaceForFace(face.self);
            if (!surface || !surface->valid() || !geometry.surface(*surface))
                ++result.missingFaceGeometry;
        }

        result.valid = result.missingEdgeGeometry == 0 &&
                       result.missingFaceGeometry == 0;
        return result;
    }
};

} // namespace mir
