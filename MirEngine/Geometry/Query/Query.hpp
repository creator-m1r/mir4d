
#pragma once

#include "../../Math/Plane.hpp"
#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace mir
{

class Line3
{
public:
    Point3 origin{};
    Vector3 direction{1.0, 0.0, 0.0};

    constexpr Line3() noexcept = default;

    constexpr Line3(const Point3& origin_, const Vector3& direction_) noexcept
        : origin(origin_)
        , direction(direction_)
    {
    }

    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept
    {
        return origin + direction * t;
    }

    [[nodiscard]] bool isValid(Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        return origin.isFinite() && direction.isFinite() && !direction.isZero(epsilon);
    }
};

class Ray3
{
public:
    Point3 origin{};
    Vector3 direction{1.0, 0.0, 0.0};

    constexpr Ray3() noexcept = default;

    constexpr Ray3(const Point3& origin_, const Vector3& direction_) noexcept
        : origin(origin_)
        , direction(direction_)
    {
    }

    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept
    {
        return origin + direction * t;
    }

    [[nodiscard]] bool isValid(Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        return origin.isFinite() && direction.isFinite() && !direction.isZero(epsilon);
    }
};

class Segment3
{
public:
    Point3 a{};
    Point3 b{1.0, 0.0, 0.0};

    constexpr Segment3() noexcept = default;

    constexpr Segment3(const Point3& a_, const Point3& b_) noexcept
        : a(a_)
        , b(b_)
    {
    }

    [[nodiscard]] Vector3 direction() const noexcept
    {
        return b - a;
    }

    [[nodiscard]] Scalar length() const noexcept
    {
        return (b - a).length();
    }

    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept
    {
        return a + (b - a) * t;
    }

    [[nodiscard]] Point3 midpoint() const noexcept
    {
        return Point3::lerp(a, b, Scalar(0.5));
    }

    [[nodiscard]] bool isValid(Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        return a.isFinite() && b.isFinite() && !direction().isZero(epsilon);
    }
};

class GeometryQuery
{
public:

    [[nodiscard]] static Point3 projectPointOnLine(const Point3& point, const Line3& line) noexcept;
    [[nodiscard]] static Point3 projectPointOnRay(const Point3& point, const Ray3& ray) noexcept;
    [[nodiscard]] static Point3 projectPointOnSegment(const Point3& point, const Segment3& segment) noexcept;

    [[nodiscard]] static Scalar distancePointToLine(const Point3& point, const Line3& line) noexcept;
    [[nodiscard]] static Scalar distancePointToRay(const Point3& point, const Ray3& ray) noexcept;
    [[nodiscard]] static Scalar distancePointToSegment(const Point3& point, const Segment3& segment) noexcept;
    [[nodiscard]] static Scalar distanceLineToLine(const Line3& first, const Line3& second) noexcept;
    [[nodiscard]] static Scalar distanceSegmentToSegment(const Segment3& first, const Segment3& second) noexcept;

    [[nodiscard]] static std::optional<Point3> intersectLinePlane(const Line3& line, const MathPlane& plane) noexcept;
    [[nodiscard]] static std::optional<Point3> intersectRayPlane(const Ray3& ray, const MathPlane& plane) noexcept;
    [[nodiscard]] static std::optional<Point3> intersectSegmentPlane(const Segment3& segment, const MathPlane& plane) noexcept;
    [[nodiscard]] static std::optional<Point3> intersectLineLine(const Line3& first, const Line3& second) noexcept;
    [[nodiscard]] static std::optional<Point3> intersectSegmentSegment(const Segment3& first, const Segment3& second) noexcept;
    [[nodiscard]] static std::optional<Point3> intersectRayTriangle(const Ray3& ray, const Point3& v0, const Point3& v1, const Point3& v2) noexcept;
};

inline Point3 GeometryQuery::projectPointOnLine(const Point3& point, const Line3& line) noexcept
{
    const Scalar t = Vector3::dot(point - line.origin, line.direction) / line.direction.lengthSquared();
    return line.pointAt(t);
}

inline Point3 GeometryQuery::projectPointOnRay(const Point3& point, const Ray3& ray) noexcept
{
    const Scalar t = std::max(Scalar(0.0), Vector3::dot(point - ray.origin, ray.direction) / ray.direction.lengthSquared());
    return ray.pointAt(t);
}

inline Point3 GeometryQuery::projectPointOnSegment(const Point3& point, const Segment3& segment) noexcept
{
    const Vector3 d = segment.direction();
    const Scalar t = std::clamp(Vector3::dot(point - segment.a, d) / d.lengthSquared(), Scalar(0.0), Scalar(1.0));
    return segment.pointAt(t);
}

inline Scalar GeometryQuery::distancePointToLine(const Point3& point, const Line3& line) noexcept
{
    return Vector3::cross(point - line.origin, line.direction).length() / line.direction.length();
}

inline Scalar GeometryQuery::distancePointToRay(const Point3& point, const Ray3& ray) noexcept
{
    const Scalar t = Vector3::dot(point - ray.origin, ray.direction) / ray.direction.lengthSquared();

    if (t <= 0.0)
    {
        return (point - ray.origin).length();
    }

    return (point - ray.pointAt(t)).length();
}

inline Scalar GeometryQuery::distancePointToSegment(const Point3& point, const Segment3& segment) noexcept
{
    return (point - projectPointOnSegment(point, segment)).length();
}

inline Scalar GeometryQuery::distanceLineToLine(const Line3& first, const Line3& second) noexcept
{
    const Vector3 n = Vector3::cross(first.direction, second.direction);
    const Scalar nLengthSquared = n.lengthSquared();

    if (nLengthSquared <= Scalar(1e-24))
    {

        return distancePointToLine(second.origin, first);
    }

    return std::abs(Vector3::dot(n, second.origin - first.origin)) / std::sqrt(nLengthSquared);
}

inline Scalar GeometryQuery::distanceSegmentToSegment(const Segment3& first, const Segment3& second) noexcept
{

    const Vector3 d1 = first.direction();
    const Vector3 d2 = second.direction();
    const Vector3 r = first.a - second.a;

    const Scalar a = d1.lengthSquared();
    const Scalar e = d2.lengthSquared();
    const Scalar f = Vector3::dot(d2, r);

    constexpr Scalar degenerate = Scalar(1e-24);

    Scalar s = 0.0;
    Scalar t = 0.0;

    if (a <= degenerate && e <= degenerate)
    {
        return (first.a - second.a).length();
    }

    if (a <= degenerate)
    {
        s = 0.0;
        t = std::clamp(f / e, Scalar(0.0), Scalar(1.0));
    }
    else
    {
        const Scalar c = Vector3::dot(d1, r);

        if (e <= degenerate)
        {
            t = 0.0;
            s = std::clamp(-c / a, Scalar(0.0), Scalar(1.0));
        }
        else
        {
            const Scalar b = Vector3::dot(d1, d2);
            const Scalar denom = a * e - b * b;

            if (denom != 0.0)
            {
                s = std::clamp((b * f - c * e) / denom, Scalar(0.0), Scalar(1.0));
            }
            else
            {
                s = 0.0;
            }

            t = (b * s + f) / e;

            if (t < 0.0)
            {
                t = 0.0;
                s = std::clamp(-c / a, Scalar(0.0), Scalar(1.0));
            }
            else if (t > 1.0)
            {
                t = 1.0;
                s = std::clamp((b - c) / a, Scalar(0.0), Scalar(1.0));
            }
        }
    }

    const Point3 closestOnFirst = first.pointAt(s);
    const Point3 closestOnSecond = second.pointAt(t);

    return (closestOnFirst - closestOnSecond).length();
}

inline std::optional<Point3> GeometryQuery::intersectLinePlane(const Line3& line, const MathPlane& plane) noexcept
{
    const Scalar denom = Vector3::dot(plane.normal, line.direction);

    if (std::abs(denom) <= Scalar(1e-12))
    {

        return std::nullopt;
    }

    const Scalar t = -plane.signedDistance(line.origin) / denom;

    if (!std::isfinite(t))
    {
        return std::nullopt;
    }

    return line.pointAt(t);
}

inline std::optional<Point3> GeometryQuery::intersectRayPlane(const Ray3& ray, const MathPlane& plane) noexcept
{
    const Scalar denom = Vector3::dot(plane.normal, ray.direction);

    if (std::abs(denom) <= Scalar(1e-12))
    {
        return std::nullopt;
    }

    const Scalar t = -plane.signedDistance(ray.origin) / denom;

    if (t < 0.0 || !std::isfinite(t))
    {
        return std::nullopt;
    }

    return ray.pointAt(t);
}

inline std::optional<Point3> GeometryQuery::intersectSegmentPlane(const Segment3& segment, const MathPlane& plane) noexcept
{
    const Scalar denom = Vector3::dot(plane.normal, segment.direction());

    if (std::abs(denom) <= Scalar(1e-12))
    {
        return std::nullopt;
    }

    const Scalar t = -plane.signedDistance(segment.a) / denom;

    if (t < 0.0 || t > 1.0 || !std::isfinite(t))
    {
        return std::nullopt;
    }

    return segment.pointAt(t);
}

inline std::optional<Point3> GeometryQuery::intersectLineLine(const Line3& first, const Line3& second) noexcept
{
    constexpr Scalar eps = Scalar(1e-9);

    const Vector3 d1 = first.direction;
    const Vector3 d2 = second.direction;
    const Vector3 n = Vector3::cross(d1, d2);
    const Scalar denom = n.lengthSquared();

    if (denom <= Scalar(1e-24))
    {

        return std::nullopt;
    }

    const Vector3 r = second.origin - first.origin;

    const Scalar t1 = Vector3::dot(Vector3::cross(r, d2), n) / denom;
    const Scalar t2 = Vector3::dot(Vector3::cross(r, d1), n) / denom;

    const Point3 p1 = first.pointAt(t1);
    const Point3 p2 = second.pointAt(t2);

    if (!p1.isFinite() || !p2.isFinite())
    {
        return std::nullopt;
    }

    if ((p1 - p2).lengthSquared() > eps * eps)
    {

        return std::nullopt;
    }

    return p1;
}

inline std::optional<Point3> GeometryQuery::intersectSegmentSegment(const Segment3& first, const Segment3& second) noexcept
{
    constexpr Scalar eps = Scalar(1e-9);

    const Vector3 d1 = first.direction();
    const Vector3 d2 = second.direction();
    const Vector3 r = first.a - second.a;

    const Scalar a = d1.lengthSquared();
    const Scalar e = d2.lengthSquared();
    const Scalar f = Vector3::dot(d2, r);

    constexpr Scalar degenerate = Scalar(1e-24);

    if (a <= degenerate || e <= degenerate)
    {
        return std::nullopt;
    }

    const Scalar b = Vector3::dot(d1, d2);
    const Scalar denom = a * e - b * b;

    if (denom <= degenerate)
    {

        const Vector3 crossCheck = Vector3::cross(d1, r);

        if (crossCheck.lengthSquared() > eps * eps * a)
        {
            return std::nullopt;
        }

        const Scalar invA = 1.0 / a;
        Scalar tA = Vector3::dot(second.a - first.a, d1) * invA;
        Scalar tB = Vector3::dot(second.b - first.a, d1) * invA;

        if (tA > tB)
        {
            std::swap(tA, tB);
        }

        const Scalar lo = std::max(Scalar(0.0), tA);
        const Scalar hi = std::min(Scalar(1.0), tB);

        if (lo > hi + eps)
        {
            return std::nullopt;
        }

        return first.pointAt(lo);
    }

    const Scalar c = Vector3::dot(d1, r);

    Scalar s = std::clamp((b * f - c * e) / denom, Scalar(0.0), Scalar(1.0));
    Scalar t = (b * s + f) / e;

    if (t < 0.0)
    {
        t = 0.0;
        s = std::clamp(-c / a, Scalar(0.0), Scalar(1.0));
    }
    else if (t > 1.0)
    {
        t = 1.0;
        s = std::clamp((b - c) / a, Scalar(0.0), Scalar(1.0));
    }

    const Point3 closestOnFirst = first.pointAt(s);
    const Point3 closestOnSecond = second.pointAt(t);

    if ((closestOnFirst - closestOnSecond).lengthSquared() > eps * eps)
    {
        return std::nullopt;
    }

    return Point3::lerp(closestOnFirst, closestOnSecond, Scalar(0.5));
}

inline std::optional<Point3> GeometryQuery::intersectRayTriangle(const Ray3& ray, const Point3& v0, const Point3& v1, const Point3& v2) noexcept
{

    constexpr Scalar eps = Scalar(1e-12);

    const Vector3 edge1 = v1 - v0;
    const Vector3 edge2 = v2 - v0;

    const Vector3 pvec = Vector3::cross(ray.direction, edge2);
    const Scalar det = Vector3::dot(edge1, pvec);

    if (std::abs(det) <= eps)
    {

        return std::nullopt;
    }

    const Scalar invDet = 1.0 / det;
    const Vector3 tvec = ray.origin - v0;

    const Scalar u = Vector3::dot(tvec, pvec) * invDet;

    if (u < -eps || u > 1.0 + eps)
    {
        return std::nullopt;
    }

    const Vector3 qvec = Vector3::cross(tvec, edge1);
    const Scalar v = Vector3::dot(ray.direction, qvec) * invDet;

    if (v < -eps || u + v > 1.0 + eps)
    {
        return std::nullopt;
    }

    const Scalar t = Vector3::dot(edge2, qvec) * invDet;

    if (t < -eps)
    {
        return std::nullopt;
    }

    return ray.pointAt(t);
}

}