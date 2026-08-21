#pragma once

#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace mir
{

struct TriangleMesh3
{
    struct Triangle
    {
        std::size_t a{0};
        std::size_t b{0};
        std::size_t c{0};

        std::uint64_t sourceFaceId{0};
    };

    std::vector<Point3> vertices;
    std::vector<Triangle> triangles;
    std::vector<Vector3> normals;

    [[nodiscard]] bool empty() const noexcept { return vertices.empty() || triangles.empty(); }
    [[nodiscard]] bool hasVertexNormals() const noexcept { return normals.size() == vertices.size(); }

    [[nodiscard]] bool isValid() const noexcept
    {
        if (vertices.empty() || triangles.empty()) return false;
        for (const Point3& p : vertices) if (!p.isFinite()) return false;
        if (!normals.empty() && normals.size() != vertices.size()) return false;
        for (const Triangle& t : triangles)
        {
            if (t.a >= vertices.size() || t.b >= vertices.size() || t.c >= vertices.size() ||
                t.a == t.b || t.b == t.c || t.a == t.c)
                return false;
        }
        return true;
    }

    [[nodiscard]] Point3 boundsMin() const noexcept
    {
        if (vertices.empty()) return Point3::origin();
        Point3 result = vertices.front();
        for (const Point3& p : vertices)
        {
            result.x = std::min(result.x, p.x);
            result.y = std::min(result.y, p.y);
            result.z = std::min(result.z, p.z);
        }
        return result;
    }

    [[nodiscard]] Point3 boundsMax() const noexcept
    {
        if (vertices.empty()) return Point3::origin();
        Point3 result = vertices.front();
        for (const Point3& p : vertices)
        {
            result.x = std::max(result.x, p.x);
            result.y = std::max(result.y, p.y);
            result.z = std::max(result.z, p.z);
        }
        return result;
    }
};

}
