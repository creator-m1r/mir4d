#pragma once

#include "../../Tessellation/TessellationMesh.hpp"
#include "../../Geometry/Tessellation/TriangleMesh.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace mir::io
{

[[nodiscard]] inline TriangleMesh3 toTriangleMesh(
    const TessellationMesh& source,
    bool generateNormals = true)
{
    TriangleMesh3 result;
    result.vertices.reserve(source.vertices.size());
    result.triangles.reserve(source.triangles.size());

    for (const auto& vertex : source.vertices)
    {
        result.vertices.push_back({
            vertex.x,
            vertex.y,
            vertex.z});

        if (generateNormals)
        {
            result.normals.push_back({
                vertex.nx,
                vertex.ny,
                vertex.nz});
        }
    }

    for (const auto& triangle : source.triangles)
    {
        result.triangles.push_back({
            static_cast<std::size_t>(triangle.a),
            static_cast<std::size_t>(triangle.b),
            static_cast<std::size_t>(triangle.c)});
    }

    if (!generateNormals)
        result.normals.clear();

    return result;
}

[[nodiscard]] inline TessellationMesh toTessellationMesh(
    const TriangleMesh3& source)
{
    TessellationMesh result;
    result.vertices.reserve(source.vertices.size());
    result.triangles.reserve(source.triangles.size());

    for (std::size_t i = 0; i < source.vertices.size(); ++i)
    {
        TessellationVertex vertex;
        vertex.x = source.vertices[i].x;
        vertex.y = source.vertices[i].y;
        vertex.z = source.vertices[i].z;

        if (i < source.normals.size())
        {
            vertex.nx = source.normals[i].x;
            vertex.ny = source.normals[i].y;
            vertex.nz = source.normals[i].z;
        }

        result.vertices.push_back(vertex);
    }

    for (const auto& triangle : source.triangles)
    {
        result.triangles.push_back({
            static_cast<std::uint32_t>(triangle.a),
            static_cast<std::uint32_t>(triangle.b),
            static_cast<std::uint32_t>(triangle.c),
            0});
    }

    return result;
}

} // namespace mir::io
