// MirEngine/Geometry/Line/Line.hpp
// Бесконечная прямая в 3D, заданная точкой и направлением.
#pragma once

#include "../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"
#include "../../Core/Types/Scalar.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace mir
{

class Line3
{
public:
    Point3 origin{};
    Vector3 direction{Vector3::unitX()};

    constexpr Line3() noexcept = default;
    Line3(const Point3& origin_, const Vector3& direction_) noexcept : origin(origin_), direction(direction_) {}

    [[nodiscard]] static Line3 fromTwoPoints(const Point3& p1, const Point3& p2) noexcept { return Line3(p1, p2 - p1); }
    [[nodiscard]] bool isValid(Scalar epsilon = 1e-12) const noexcept { return direction.isFinite() && direction.lengthSquared() > epsilon * epsilon; }
    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept { return origin + direction * t; }
    [[nodiscard]] Scalar parameterOf(const Point3& point) const noexcept
    {
        const Scalar lengthSq = direction.lengthSquared();
        if (lengthSq <= Scalar(1e-20)) return 0.0;
        return Vector3::dot(point - origin, direction) / lengthSq;
    }
    [[nodiscard]] Point3 project(const Point3& point) const noexcept { return pointAt(parameterOf(point)); }
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = 1e-10) const noexcept
    {
        if (!isValid()) return Point3::distance(point, origin) <= tolerance;
        const Vector3 offset = point - origin;
        return Vector3::cross(offset, direction).length() <= tolerance * direction.length();
    }
    [[nodiscard]] Scalar distanceTo(const Point3& point) const noexcept
    {
        if (!isValid()) return Point3::distance(point, origin);
        const Vector3 offset = point - origin;
        return Vector3::cross(offset, direction).length() / direction.length();
    }

    [[nodiscard]] static std::pair<Point3, Point3> closestPoints(const Line3& a, const Line3& b) noexcept
    {
        const Scalar eps = 1e-20;
        const Scalar aLenSq = a.direction.lengthSquared();
        const Scalar bLenSq = b.direction.lengthSquared();
        if (aLenSq <= eps && bLenSq <= eps) return {a.origin, b.origin};
        if (aLenSq <= eps) return {a.origin, b.project(a.origin)};
        if (bLenSq <= eps) return {a.project(b.origin), b.origin};
        const Vector3 r = a.origin - b.origin;
        const Scalar ab = Vector3::dot(a.direction, b.direction);
        const Scalar ar = Vector3::dot(a.direction, r);
        const Scalar br = Vector3::dot(b.direction, r);
        const Scalar denom = aLenSq * bLenSq - ab * ab;
        if (std::abs(denom) <= eps) return {a.origin, b.project(a.origin)};
        const Scalar t1 = (ab * br - bLenSq * ar) / denom;
        const Scalar t2 = (aLenSq * br - ab * ar) / denom;
        return {a.pointAt(t1), b.pointAt(t2)};
    }

    [[nodiscard]] static Scalar distanceBetween(const Line3& a, const Line3& b) noexcept
    {
        const auto [p1, p2] = closestPoints(a, b);
        return Point3::distance(p1, p2);
    }

    friend bool operator==(const Line3& a, const Line3& b) noexcept
    {
        if (!a.isValid() || !b.isValid()) return a.origin == b.origin && a.direction == b.direction;
        const Vector3 cross = Vector3::cross(a.direction, b.direction);
        const Scalar scale = a.direction.length() * b.direction.length();
        return cross.length() <= Scalar(1e-10) * scale && a.contains(b.origin);
    }
    friend bool operator!=(const Line3& a, const Line3& b) noexcept { return !(a == b); }
};

} // namespace mir
