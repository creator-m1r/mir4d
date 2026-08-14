#pragma once

#include "../../Tessellation/TessellationMesh.hpp"

#include <cstdint>
#include <vector>

namespace MirEngine {
namespace Rendering {

struct RenderVertex
{
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};
    float nx{0.0F};
    float ny{0.0F};
    float nz{1.0F};
};

struct RenderTriangle
{
    std::uint32_t a{0};
    std::uint32_t b{0};
    std::uint32_t c{0};
    std::uint64_t sourceFaceId{0};
};

struct RenderMesh
{
    std::vector<RenderVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<RenderTriangle> triangles;

    [[nodiscard]] bool empty() const noexcept
    {
        return vertices.empty() || indices.empty();
    }
};

class RenderMeshConverter
{
public:
    [[nodiscard]] static RenderMesh fromTessellation(
        const ::mir::TessellationMesh& mesh) noexcept
    {
        RenderMesh result;
        result.vertices.reserve(mesh.vertices.size());
        result.indices.reserve(mesh.triangles.size() * 3);
        result.triangles.reserve(mesh.triangles.size());

        for (const auto& vertex : mesh.vertices)
        {
            result.vertices.push_back({
                static_cast<float>(vertex.x),
                static_cast<float>(vertex.y),
                static_cast<float>(vertex.z),
                static_cast<float>(vertex.nx),
                static_cast<float>(vertex.ny),
                static_cast<float>(vertex.nz)});
        }

        for (const auto& triangle : mesh.triangles)
        {
            result.indices.push_back(triangle.a);
            result.indices.push_back(triangle.b);
            result.indices.push_back(triangle.c);
            result.triangles.push_back({
                triangle.a,
                triangle.b,
                triangle.c,
                triangle.sourceFaceId});
        }

        return result;
    }
};

} // namespace Rendering
} // namespace MirEngine
