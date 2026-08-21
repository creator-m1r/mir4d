#pragma once

#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace mir
{

/// Sentinel stored in `Triangle::sourceEdgeId[k]` when the triangle edge does
/// not correspond to a source B-Rep edge (e.g. an internal tessellation seam).
static constexpr std::uint64_t kInvalidSourceEdge = std::numeric_limits<std::uint64_t>::max();

/// Render-oriented triangle mesh. CAD topology remains the source of truth;
/// this type stores tessellated geometry for rendering, selection and export.
struct TriangleMesh3
{
    struct Triangle
    {
        std::size_t a{0};
        std::size_t b{0};
        std::size_t c{0};

        // Stable source B-Rep face identifier (BRepFaceHandle::index).
        // Zero means that provenance is not available.
        std::uint64_t sourceFaceId{0};

        // Stable source B-Rep edge identifiers for the three triangle edges
        // (a-b, b-c, c-a). A value of kInvalidSourceEdge means that edge is an
        // internal tessellation seam, not a real CAD edge.
        std::uint64_t sourceEdgeId[3]{
            kInvalidSourceEdge, kInvalidSourceEdge, kInvalidSourceEdge};
    };

    std::vector<Point3> vertices;
    std::vector<Triangle> triangles;
    std::vector<Vector3> normals;

    /// Monotonic geometry epoch. Bumped whenever the vertex/triangle data is
    /// replaced or mutated in place, so spatial acceleration structures cached
    /// against this mesh can detect staleness. Starts at zero for a fresh mesh.
    mutable std::uint32_t geometryEpoch{0};

    /// Signals that the geometry changed; any cached acceleration structure
    /// keyed on this mesh must be rebuilt. Safe to call on a const instance
    /// because the epoch is mutable.
    void markGeometryChanged() const noexcept { ++geometryEpoch; }

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

} // namespace mir
