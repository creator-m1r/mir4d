#include "RenderSelectionProperties.hpp"

#include <cmath>

namespace MirEngine {
namespace Rendering {
namespace
{

[[nodiscard]] RenderVec3 subtract(const RenderVec3& a, const RenderVec3& b) noexcept
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

[[nodiscard]] RenderVec3 cross(const RenderVec3& a, const RenderVec3& b) noexcept
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

[[nodiscard]] double length(const RenderVec3& value) noexcept
{
    return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

[[nodiscard]] RenderVec3 normalize(const RenderVec3& value) noexcept
{
    const double magnitude = length(value);
    if (magnitude <= 1e-12)
        return {};

    return {
        value.x / magnitude,
        value.y / magnitude,
        value.z / magnitude};
}

} // namespace

RenderSelectionProperties RenderSelectionPropertiesBuilder::build(
    const RenderSelection& selection,
    const RenderMesh& mesh) noexcept
{
    RenderSelectionProperties result;
    result.selection = selection;

    if (!selection.valid() || selection.type != RenderSelectionType::Face)
        return result;

    RenderVec3 weightedCenter{};
    RenderVec3 accumulatedNormal{};
    double totalArea = 0.0;

    for (const auto& triangle : mesh.triangles)
    {
        if (triangle.sourceFaceId != selection.id)
            continue;

        if (triangle.a >= mesh.vertices.size() ||
            triangle.b >= mesh.vertices.size() ||
            triangle.c >= mesh.vertices.size())
            continue;

        const auto& va = mesh.vertices[triangle.a];
        const auto& vb = mesh.vertices[triangle.b];
        const auto& vc = mesh.vertices[triangle.c];

        const RenderVec3 a{va.x, va.y, va.z};
        const RenderVec3 b{vb.x, vb.y, vb.z};
        const RenderVec3 c{vc.x, vc.y, vc.z};

        const auto crossProduct = cross(subtract(b, a), subtract(c, a));
        const double triangleArea = 0.5 * length(crossProduct);
        if (triangleArea <= 1e-12)
            continue;

        const RenderVec3 centroid{
            (a.x + b.x + c.x) / 3.0,
            (a.y + b.y + c.y) / 3.0,
            (a.z + b.z + c.z) / 3.0};

        weightedCenter.x += centroid.x * triangleArea;
        weightedCenter.y += centroid.y * triangleArea;
        weightedCenter.z += centroid.z * triangleArea;
        accumulatedNormal.x += crossProduct.x;
        accumulatedNormal.y += crossProduct.y;
        accumulatedNormal.z += crossProduct.z;
        totalArea += triangleArea;
        ++result.triangleCount;
    }

    if (totalArea <= 1e-12)
        return result;

    result.area = totalArea;
    result.center = {
        weightedCenter.x / totalArea,
        weightedCenter.y / totalArea,
        weightedCenter.z / totalArea};
    result.normal = normalize(accumulatedNormal);
    return result;
}

} // namespace Rendering
} // namespace MirEngine
