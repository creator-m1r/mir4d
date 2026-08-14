#pragma once

#include "RenderRay.hpp"
#include "RenderSelection.hpp"
#include "../Resources/RenderMesh.hpp"

#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

namespace MirEngine {
namespace Rendering {

struct RenderHit
{
    std::uint32_t triangleIndex{0};
    std::uint64_t sourceFaceId{0};
    double distance{std::numeric_limits<double>::max()};
    RenderVec3 position{};

    [[nodiscard]] RenderSelection selection() const noexcept
    {
        if (sourceFaceId == 0)
            return RenderSelection::none();

        return {RenderSelectionType::Face, sourceFaceId};
    }
};

class RayIntersector
{
public:
    [[nodiscard]] static std::optional<RenderHit> intersect(
        const RenderRay& ray,
        const RenderMesh& mesh) noexcept
    {
        std::optional<RenderHit> closest;

        for (std::uint32_t triangleIndex = 0;
             triangleIndex < mesh.triangles.size();
             ++triangleIndex)
        {
            const auto& triangle = mesh.triangles[triangleIndex];

            if (triangle.a >= mesh.vertices.size() ||
                triangle.b >= mesh.vertices.size() ||
                triangle.c >= mesh.vertices.size())
                continue;

            const RenderVec3 a{mesh.vertices[triangle.a].x, mesh.vertices[triangle.a].y, mesh.vertices[triangle.a].z};
            const RenderVec3 b{mesh.vertices[triangle.b].x, mesh.vertices[triangle.b].y, mesh.vertices[triangle.b].z};
            const RenderVec3 c{mesh.vertices[triangle.c].x, mesh.vertices[triangle.c].y, mesh.vertices[triangle.c].z};

            const auto hit = intersectTriangle(ray, a, b, c);
            if (!hit)
                continue;

            RenderHit candidate{};
            candidate.triangleIndex = triangleIndex;
            candidate.sourceFaceId = triangle.sourceFaceId;
            candidate.distance = hit->first;
            candidate.position = hit->second;

            if (!closest || candidate.distance < closest->distance)
                closest = candidate;
        }

        return closest;
    }

private:
    [[nodiscard]] static std::optional<std::pair<double, RenderVec3>> intersectTriangle(
        const RenderRay& ray,
        const RenderVec3& a,
        const RenderVec3& b,
        const RenderVec3& c) noexcept
    {
        const RenderVec3 edge1{b.x - a.x, b.y - a.y, b.z - a.z};
        const RenderVec3 edge2{c.x - a.x, c.y - a.y, c.z - a.z};
        const RenderVec3 pvec{
            ray.direction.y * edge2.z - ray.direction.z * edge2.y,
            ray.direction.z * edge2.x - ray.direction.x * edge2.z,
            ray.direction.x * edge2.y - ray.direction.y * edge2.x};

        const double determinant =
            edge1.x * pvec.x + edge1.y * pvec.y + edge1.z * pvec.z;

        constexpr double epsilon = 1e-10;
        if (determinant > -epsilon && determinant < epsilon)
            return std::nullopt;

        const double inverseDeterminant = 1.0 / determinant;
        const RenderVec3 tvec{
            ray.origin.x - a.x,
            ray.origin.y - a.y,
            ray.origin.z - a.z};

        const double u =
            (tvec.x * pvec.x + tvec.y * pvec.y + tvec.z * pvec.z) *
            inverseDeterminant;

        if (u < 0.0 || u > 1.0)
            return std::nullopt;

        const RenderVec3 qvec{
            tvec.y * edge1.z - tvec.z * edge1.y,
            tvec.z * edge1.x - tvec.x * edge1.z,
            tvec.x * edge1.y - tvec.y * edge1.x};

        const double v =
            (ray.direction.x * qvec.x +
             ray.direction.y * qvec.y +
             ray.direction.z * qvec.z) *
            inverseDeterminant;

        if (v < 0.0 || u + v > 1.0)
            return std::nullopt;

        const double distance =
            (edge2.x * qvec.x + edge2.y * qvec.y + edge2.z * qvec.z) *
            inverseDeterminant;

        if (distance <= epsilon)
            return std::nullopt;

        return std::make_pair(
            distance,
            RenderVec3{
                ray.origin.x + ray.direction.x * distance,
                ray.origin.y + ray.direction.y * distance,
                ray.origin.z + ray.direction.z * distance});
    }
};

} // namespace Rendering
} // namespace MirEngine
