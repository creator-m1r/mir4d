// MirEngine/Math/Plane.hpp
//
// Lightweight mathematical plane equation used by numerical algorithms.
//
// The scene/geometry plane is MirEngine/Geometry/Plane/Plane.hpp and owns
// geometric concepts such as Point3 and Direction3.  This type intentionally
// remains independent from Geometry so low-level Math code does not depend on
// the higher geometry layer.
//
// Representation:
//     dot(normal, point) + d = 0
//
// The normal is kept normalized by the constructors and normalize().
// C++23, no external dependencies.

#pragma once

#include "Vector/Vector3.hpp"
#include "../Core/Types/Scalar.hpp"

#include <cmath>

namespace mir {

class MathPlane {
public:
    Vector3 normal{0.0, 0.0, 1.0};
    Scalar d{0.0};

    constexpr MathPlane() noexcept = default;

    MathPlane(const Vector3& n, const Vector3& point) noexcept
        : normal(n.normalized())
        , d(-Vector3::dot(normal, point))
    {
    }

    static MathPlane fromPoints(
        const Vector3& p1,
        const Vector3& p2,
        const Vector3& p3) noexcept
    {
        const Vector3 n = Vector3::cross(p2 - p1, p3 - p1).normalized();
        return MathPlane(n, p1);
    }

    [[nodiscard]] Scalar signedDistance(const Vector3& point) const noexcept
    {
        return Vector3::dot(normal, point) + d;
    }

    [[nodiscard]] Scalar distance(const Vector3& point) const noexcept
    {
        return std::abs(signedDistance(point));
    }

    [[nodiscard]] Vector3 project(const Vector3& point) const noexcept
    {
        return point - normal * signedDistance(point);
    }

    [[nodiscard]] bool isOnPositiveSide(
        const Vector3& point,
        Scalar tolerance = Scalar(1e-10)) const noexcept
    {
        return signedDistance(point) > tolerance;
    }

    void normalize() noexcept
    {
        const Scalar len = normal.length();

        if (len > Scalar(1e-20)) {
            normal /= len;
            d /= len;
        }
    }

    friend constexpr bool operator==(
        const MathPlane& a,
        const MathPlane& b) noexcept
    {
        return a.normal == b.normal && a.d == b.d;
    }

    friend constexpr bool operator!=(
        const MathPlane& a,
        const MathPlane& b) noexcept
    {
        return !(a == b);
    }
};

} // namespace mir
