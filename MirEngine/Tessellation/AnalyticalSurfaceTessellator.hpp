#pragma once

#include "TessellationMesh.hpp"
#include "MirEngine/BRep/Geometry/BRepGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <variant>

namespace mir
{

class AnalyticalSurfaceTessellator
{
public:
    [[nodiscard]] static TessellationMesh tessellate(
        const BRepSurface& surface,
        std::uint32_t segments = 32,
        double extent = 1.0) noexcept
    {
        TessellationMesh mesh;
        segments = std::max<std::uint32_t>(segments, 3);
        extent = std::max(extent, 1e-6);
        std::visit([&](const auto& geometry) { tessellateSurface(geometry, segments, extent, mesh); }, surface);
        return mesh;
    }

private:
    static void tessellateSurface(const BRepPlane& plane, std::uint32_t, double extent, TessellationMesh& mesh) noexcept
    {
        const auto base = mesh.vertices.size();
        mesh.vertices.push_back({plane.origin.x - extent, plane.origin.y - extent, plane.origin.z,
                                 plane.normal.x, plane.normal.y, plane.normal.z});
        mesh.vertices.push_back({plane.origin.x + extent, plane.origin.y - extent, plane.origin.z,
                                 plane.normal.x, plane.normal.y, plane.normal.z});
        mesh.vertices.push_back({plane.origin.x + extent, plane.origin.y + extent, plane.origin.z,
                                 plane.normal.x, plane.normal.y, plane.normal.z});
        mesh.vertices.push_back({plane.origin.x - extent, plane.origin.y + extent, plane.origin.z,
                                 plane.normal.x, plane.normal.y, plane.normal.z});
        mesh.triangles.push_back({static_cast<std::uint32_t>(base), static_cast<std::uint32_t>(base + 1), static_cast<std::uint32_t>(base + 2)});
        mesh.triangles.push_back({static_cast<std::uint32_t>(base), static_cast<std::uint32_t>(base + 2), static_cast<std::uint32_t>(base + 3)});
    }

    static void tessellateSurface(const BRepCylinder& cylinder, std::uint32_t segments, double extent, TessellationMesh& mesh) noexcept
    {
        const double radius = std::max(cylinder.radius, 1e-6);
        const double height = std::max(extent, 1e-6);
        const auto base = static_cast<std::uint32_t>(mesh.vertices.size());
        for (std::uint32_t row = 0; row < 2; ++row)
        {
            const double z = row == 0 ? 0.0 : height;
            for (std::uint32_t i = 0; i < segments; ++i)
            {
                const double angle = 2.0 * 3.14159265358979323846 * static_cast<double>(i) / static_cast<double>(segments);
                const double nx = std::cos(angle);
                const double ny = std::sin(angle);
                mesh.vertices.push_back({cylinder.origin.x + radius * nx, cylinder.origin.y + radius * ny, cylinder.origin.z + z,
                                         nx, ny, 0.0});
            }
        }
        for (std::uint32_t i = 0; i < segments; ++i)
        {
            const auto next = (i + 1) % segments;
            const auto b0 = base + i;
            const auto b1 = base + next;
            const auto t0 = base + segments + i;
            const auto t1 = base + segments + next;
            mesh.triangles.push_back({b0, b1, t1});
            mesh.triangles.push_back({b0, t1, t0});
        }
    }
};

} // namespace mir
