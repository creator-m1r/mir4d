// MirEngine/Geometry/Ray/Ray.hpp
// Полубесконечный луч в 3D: origin + t * direction, t >= 0.
#pragma once

#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"
#include "../../Core/Types/Scalar.hpp"

#include <cmath>
#include <optional>

namespace mir
{

class Ray3
{
public:
    Point3 origin{};
    Vector3 direction{Vector3::unitX()};

    constexpr Ray3() noexcept = default;
    constexpr Ray3(const Point3& origin_, const Vector3& direction_) noexcept : origin(origin_), direction(direction_) {}

    [[nodiscard]] bool isValid(Scalar epsilon = 1e-12) const noexcept
    { return direction.isFinite() && direction.lengthSquared() > epsilon * epsilon; }
    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept { return origin + direction * t; }
    [[nodiscard]] Point3 closestPoint(const Point3& point, Scalar tolerance = 1e-10) const noexcept
    {
        const Scalar lengthSq = direction.lengthSquared();
        if (lengthSq <= tolerance * tolerance) return origin;
        const Scalar t = Vector3::dot(point - origin, direction) / lengthSq;
        return pointAt(t < 0.0 ? 0.0 : t);
    }
    [[nodiscard]] Scalar distanceTo(const Point3& point, Scalar tolerance = 1e-10) const noexcept
    { return Point3::distance(point, closestPoint(point, tolerance)); }
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = 1e-10) const noexcept
    {
        if (!isValid(tolerance)) return Point3::distance(point, origin) <= tolerance;
        const Vector3 offset = point - origin;
        const Scalar projection = Vector3::dot(offset, direction);
        if (projection < -tolerance * direction.length()) return false;
        return Vector3::cross(offset, direction).length() <= tolerance * direction.length();
    }
    [[nodiscard]] std::optional<Point3> intersectPlane(const Point3& planePoint, const Vector3& planeNormal, Scalar tolerance = 1e-10) const noexcept
    {
        const Scalar denominator = Vector3::dot(direction, planeNormal);
        if (std::abs(denominator) <= tolerance) return std::nullopt;
        const Scalar t = Vector3::dot(planePoint - origin, planeNormal) / denominator;
        if (t < -tolerance) return std::nullopt;
        return pointAt(t < 0.0 ? 0.0 : t);
    }

    friend constexpr bool operator==(const Ray3& a, const Ray3& b) noexcept { return a.origin == b.origin && a.direction == b.direction; }
    friend constexpr bool operator!=(const Ray3& a, const Ray3& b) noexcept { return !(a == b); }
};

} // namespace mir
