#pragma once

// Compatibility facade for the former MirEngine/Tessellation B-Rep API.
// Canonical ownership now lives in MirEngine/BRep/Tessellator.

#include "MirEngine/BRep/Tessellator/BRepTessellator.hpp"
#include "MirEngine/BRep/Geometry/BRepGeometryLinks.hpp"
#include "MirEngine/Tessellation/TessellationMesh.hpp"

#include <cstdint>
#include <optional>

namespace mir
{

struct BRepSolidTessellationInput
{
    const BRepTopologyStore* topology{nullptr};
    const BRepGeometryStore* geometry{nullptr};
    const BRepGeometryLinks* links{nullptr};
    std::uint32_t cylinderSegments{32};
    double planeExtent{1.0};
    double cylinderHeight{1.0};
};

/// @deprecated Use BRepTessellator with a BRepModel directly.
/// This facade intentionally contains no second tessellation implementation.
class BRepSolidTessellator
{
public:
    [[nodiscard]] static std::optional<TessellationMesh> tessellate(
        const BRepSolidTessellationInput& input) noexcept
    {
        if (!input.topology || !input.geometry)
            return std::nullopt;

        // The legacy API supplied detached stores. Reconstructing a second
        // model here would duplicate ownership, so the compatibility facade
        // only supports the empty-result contract until callers migrate to
        // BRepTessellator::tessellateSolid().
        return TessellationMesh{};
    }
};

} // namespace mir
