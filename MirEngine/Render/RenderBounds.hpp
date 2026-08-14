#pragma once

#include "RenderCamera.hpp"
#include "RenderMesh.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace mir
{

struct RenderBounds
{
    RenderVec3 min{
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max()};
    RenderVec3 max{
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest()};

    [[nodiscard]] bool valid() const noexcept
    {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }

    void include(RenderVec3 point) noexcept
    {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    [[nodiscard]] RenderVec3 center() const noexcept
    {
        return {
            (min.x + max.x) * 0.5,
            (min.y + max.y) * 0.5,
            (min.z + max.z) * 0.5};
    }

    [[nodiscard]] double radius() const noexcept
    {
        const auto c = center();
        const double dx = max.x - c.x;
        const double dy = max.y - c.y;
        const double dz = max.z - c.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    [[nodiscard]] static RenderBounds fromMesh(const RenderMesh& mesh) noexcept
    {
        RenderBounds result;
        for (const auto& vertex : mesh.vertices)
            result.include({vertex.x, vertex.y, vertex.z});
        return result;
    }
};

} // namespace mir
