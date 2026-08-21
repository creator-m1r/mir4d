#pragma once

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

class BRepSolidTessellator
{
public:
    [[nodiscard]] static std::optional<TessellationMesh> tessellate(
        const BRepSolidTessellationInput& input) noexcept
    {
        if (!input.topology || !input.geometry)
            return std::nullopt;

        return TessellationMesh{};
    }
};

}
