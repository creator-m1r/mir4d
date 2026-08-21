
#pragma once

#include "../../Core/Types/Scalar.hpp"
#include "../Vector/Vector3.hpp"
#include "../TransformMatrix.hpp"
#include <algorithm>

namespace mir {

class AABB {
public:

    Vector3 min;
    Vector3 max;

    constexpr AABB() noexcept
        : min( std::numeric_limits<Scalar>::max(),
               std::numeric_limits<Scalar>::max(),
               std::numeric_limits<Scalar>::max())
        , max(-std::numeric_limits<Scalar>::max(),
              -std::numeric_limits<Scalar>::max(),
              -std::numeric_limits<Scalar>::max())
    {}

    constexpr AABB(const Vector3& minPoint, const Vector3& maxPoint) noexcept
        : min(minPoint), max(maxPoint)
    {}

    [[nodiscard]] constexpr Vector3 center() const noexcept {
        return {
            (min.x + max.x) * 0.5,
            (min.y + max.y) * 0.5,
            (min.z + max.z) * 0.5
        };
    }

    [[nodiscard]] constexpr Vector3 size() const noexcept {
        return {
            max.x - min.x,
            max.y - min.y,
            max.z - min.z
        };
    }

    void extend(const Vector3& point) noexcept {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    void extend(const AABB& other) noexcept {
        extend(other.min);
        extend(other.max);
    }

    [[nodiscard]] constexpr bool contains(const Vector3& point) const noexcept {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    [[nodiscard]] constexpr bool intersects(const AABB& other) const noexcept {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }

    [[nodiscard]] AABB transformed(const Matrix4& matrix) const noexcept {

        Vector3 corners[8] = {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {min.x, max.y, min.z},
            {min.x, min.y, max.z},
            {max.x, max.y, min.z},
            {max.x, min.y, max.z},
            {min.x, max.y, max.z},
            {max.x, max.y, max.z}
        };

        AABB result;
        for (const auto& corner : corners) {
            result.extend(matrix.transformPoint(corner));
        }
        return result;
    }

    friend constexpr bool operator==(const AABB& a, const AABB& b) noexcept {
        return a.min == b.min && a.max == b.max;
    }
    friend constexpr bool operator!=(const AABB& a, const AABB& b) noexcept {
        return !(a == b);
    }
};

}