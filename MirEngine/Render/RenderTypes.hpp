// MirEngine/Render/RenderTypes.hpp
// Canonical lightweight types shared by the render layer.
// C++23

#pragma once

namespace mir
{

/// Lightweight render-layer 3D vector.
/// Independent from the canonical geometry Vector3 / Point3 types.
struct RenderVec3
{
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

/// Lightweight 4x4 render matrix.
struct RenderMat4
{
    double m[16]{};

    [[nodiscard]] static RenderMat4 identity() noexcept
    {
        RenderMat4 result{};
        result.m[0] = 1.0;
        result.m[5] = 1.0;
        result.m[10] = 1.0;
        result.m[15] = 1.0;
        return result;
    }
};

} // namespace mir
