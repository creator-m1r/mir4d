// MirEngine/Geometry/Plane/Plane.hpp
// 📐 Плоскость в трёхмерном пространстве.
//
// Геометрический примитив МИР 4D, заданный точкой и единичной нормалью.
// C++23.

#pragma once

#include "../Point/Point3.hpp"
#include "../Direction/Direction3.hpp"
#include "../Line/Line3.hpp"
#include "../Ray/Ray3.hpp"
#include "../Segment/Segment3.hpp"
#include "../../Math/Vector/Vector3.hpp"
#include "../../Core/Types/Scalar.hpp"

#include <cmath>
#include <optional>

namespace mir
{

class Plane
{
public:
    Point3 origin{};
    Direction3 normal{};

    constexpr Plane(
        const Point3& origin_,
        const Direction3& normal_) noexcept
        : origin(origin_)
        , normal(normal_)
    {
    }

    [[nodiscard]] static Plane fromThreePoints(
        const Point3& p1,
        const Point3& p2,
        const Point3& p3,
        Scalar epsilon = Scalar(1e-12)) noexcept
    {
        const Vector3 a = p2 - p1;
        const Vector3 b = p3 - p1;
        const Vector3 n = Vector3::cross(a, b);

        if (n.lengthSquared() <= epsilon * epsilon)
        {
            return Plane(p1, Direction3::unitZ());
        }

        return Plane(p1, Direction3::fromVector(n));
    }

    [[nodiscard]] Plane normalized() const noexcept
    {
        return *this;
    }

    [[nodiscard]] Scalar signedDistance(
        const Point3& point) const noexcept
    {
        return Vector3::dot(point - origin, normal.asVector());
    }

    [[nodiscard]] Scalar distance(
        const Point3& point) const noexcept
    {
        return std::abs(signedDistance(point));
    }

    [[nodiscard]] Point3 project(
        const Point3& point) const noexcept
    {
        return point - normal.asVector() * signedDistance(point);
    }

    [[nodiscard]] bool contains(
        const Point3& point,
        Scalar tolerance = Scalar(1e-10)) const noexcept
    {
        return distance(point) <= tolerance;
    }

    [[nodiscard]] std::optional<Point3> intersect(
        const Line3& line,
        Scalar tolerance = Scalar(1e-12)) const noexcept
    {
        if (!line.isValid(tolerance))
        {
            return contains(line.origin, tolerance)
                ? std::optional<Point3>(line.origin)
                : std::nullopt;
        }

        const Vector3 n = normal.asVector();
        const Scalar denominator = Vector3::dot(line.direction, n);

        if (std::abs(denominator) <= tolerance)
        {
            return std::nullopt;
        }

        const Scalar t = Vector3::dot(origin - line.origin, n) /
                         denominator;

        return line.pointAt(t);
    }

    [[nodiscard]] std::optional<Point3> intersect(
        const Ray3& ray,
        Scalar tolerance = Scalar(1e-12)) const noexcept
    {
        const Scalar directionLengthSq = ray.direction.lengthSquared();

        if (directionLengthSq <= tolerance * tolerance)
        {
            return contains(ray.origin, tolerance)
                ? std::optional<Point3>(ray.origin)
                : std::nullopt;
        }

        const Vector3 n = normal.asVector();
        const Scalar denominator = Vector3::dot(ray.direction, n);

        if (std::abs(denominator) <= tolerance)
        {
            return std::nullopt;
        }

        const Scalar t = Vector3::dot(origin - ray.origin, n) /
                         denominator;

        if (t < -tolerance)
        {
            return std::nullopt;
        }

        return ray.pointAt(t < Scalar(0) ? Scalar(0) : t);
    }

    [[nodiscard]] std::optional<Point3> intersect(
        const Segment3& segment,
        Scalar tolerance = Scalar(1e-12)) const noexcept
    {
        const Scalar d1 = signedDistance(segment.start);
        const Scalar d2 = signedDistance(segment.end);

        if (std::abs(d1) <= tolerance)
        {
            if (std::abs(d2) <= tolerance)
            {
                return segment.start;
            }

            return segment.start;
        }

        if (std::abs(d2) <= tolerance)
        {
            return segment.end;
        }

        const Scalar denominator = d1 - d2;

        if (std::abs(denominator) <= tolerance)
        {
            return std::nullopt;
        }

        const Scalar t = d1 / denominator;

        if (t < -tolerance || t > Scalar(1) + tolerance)
        {
            return std::nullopt;
        }

        const Scalar clampedT =
            t < Scalar(0) ? Scalar(0) :
            t > Scalar(1) ? Scalar(1) : t;

        return segment.pointAt(clampedT);
    }

    [[nodiscard]] int sideOf(
        const Point3& point,
        Scalar tolerance = Scalar(1e-10)) const noexcept
    {
        const Scalar d = signedDistance(point);

        if (d > tolerance)
        {
            return 1;
        }

        if (d < -tolerance)
        {
            return -1;
        }

        return 0;
    }

    [[nodiscard]] Point3 mirror(
        const Point3& point) const noexcept
    {
        return point - normal.asVector() *
                       (Scalar(2) * signedDistance(point));
    }

    [[nodiscard]] bool isFinite() const noexcept
    {
        return origin.isFinite() && normal.isFinite();
    }

    friend bool operator==(
        const Plane& a,
        const Plane& b) noexcept
    {
        const Vector3 na = a.normal.asVector();
        const Vector3 nb = b.normal.asVector();

        const Vector3 cross = Vector3::cross(na, nb);

        if (cross.lengthSquared() > Scalar(1e-20))
        {
            return false;
        }

        return a.contains(b.origin) ||
               a.contains(b.origin + nb);
    }

    friend bool operator!=(
        const Plane& a,
        const Plane& b) noexcept
    {
        return !(a == b);
    }
};

} // namespace mir
