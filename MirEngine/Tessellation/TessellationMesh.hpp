#pragma once

#include <cstdint>
#include <vector>

namespace mir
{

struct TessellationVertex
{
    double x{0.0};
    double y{0.0};
    double z{0.0};
    double nx{0.0};
    double ny{0.0};
    double nz{1.0};
};

struct TessellationTriangle
{
    std::uint32_t a{0};
    std::uint32_t b{0};
    std::uint32_t c{0};

    // Stable source B-Rep face identifier.
    // Zero means that provenance is not available.
    std::uint64_t sourceFaceId{0};
};

struct TessellationMesh
{
    std::vector<TessellationVertex> vertices;
    std::vector<TessellationTriangle> triangles;

    [[nodiscard]] bool empty() const noexcept
    {
        return vertices.empty() || triangles.empty();
    }
};

} // namespace mir
