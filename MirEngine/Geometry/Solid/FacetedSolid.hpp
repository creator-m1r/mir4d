#pragma once

#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace mir
{

/// Minimal faceted boundary representation used by the current kernel.
class Solid3
{
public:
    struct Triangle
    {
        std::size_t a{0};
        std::size_t b{0};
        std::size_t c{0};
    };

    Solid3() = default;
    Solid3(std::vector<Point3> vertices, std::vector<Triangle> triangles)
        : vertices_(std::move(vertices)), triangles_(std::move(triangles)) {}

    [[nodiscard]] const std::vector<Point3>& vertices() const noexcept { return vertices_; }
    [[nodiscard]] const std::vector<Triangle>& triangles() const noexcept { return triangles_; }
    [[nodiscard]] bool empty() const noexcept { return vertices_.empty() || triangles_.empty(); }

    [[nodiscard]] bool isValid() const noexcept
    {
        if (vertices_.empty() || triangles_.empty()) return false;
        for (const Point3& p : vertices_) if (!p.isFinite()) return false;
        for (const Triangle& t : triangles_)
        {
            if (t.a >= vertices_.size() || t.b >= vertices_.size() || t.c >= vertices_.size() ||
                t.a == t.b || t.b == t.c || t.a == t.c)
                return false;
        }
        return true;
    }

    [[nodiscard]] Point3 boundsMin() const noexcept
    {
        if (vertices_.empty()) return Point3::origin();
        Point3 result = vertices_.front();
        for (const Point3& p : vertices_)
        {
            result.x = std::min(result.x, p.x);
            result.y = std::min(result.y, p.y);
            result.z = std::min(result.z, p.z);
        }
        return result;
    }

    [[nodiscard]] Point3 boundsMax() const noexcept
    {
        if (vertices_.empty()) return Point3::origin();
        Point3 result = vertices_.front();
        for (const Point3& p : vertices_)
        {
            result.x = std::max(result.x, p.x);
            result.y = std::max(result.y, p.y);
            result.z = std::max(result.z, p.z);
        }
        return result;
    }

private:
    std::vector<Point3> vertices_;
    std::vector<Triangle> triangles_;
};

} // namespace mir
