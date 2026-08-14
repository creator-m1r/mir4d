// MirEngine/Geometry/Segment/Segment.hpp
// Конечный отрезок в 3D между start и end.
#pragma once

#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"
#include "../../Core/Types/Scalar.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

namespace mir
{

class Segment3
{
public:
    Point3 start{};
    Point3 end{};

    constexpr Segment3() noexcept = default;
    constexpr Segment3(const Point3& start_, const Point3& end_) noexcept : start(start_), end(end_) {}

    [[nodiscard]] Vector3 vector() const noexcept { return end - start; }
    [[nodiscard]] Vector3 direction() const noexcept
    {
        const Vector3 v = vector();
        const Scalar lenSq = v.lengthSquared();
        return lenSq <= Scalar(1e-20) ? Vector3::zero() : v / std::sqrt(lenSq);
    }
    [[nodiscard]] Scalar length() const noexcept { return vector().length(); }
    [[nodiscard]] Scalar lengthSquared() const noexcept { return vector().lengthSquared(); }
    [[nodiscard]] Point3 center() const noexcept { return Point3{(start.x + end.x)*0.5, (start.y + end.y)*0.5, (start.z + end.z)*0.5}; }
    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept { return start + vector() * t; }
    [[nodiscard]] Point3 closestPoint(const Point3& point) const noexcept
    {
        const Vector3 v = vector();
        const Scalar lenSq = v.lengthSquared();
        if (lenSq <= Scalar(1e-20)) return start;
        const Scalar t = std::clamp(Vector3::dot(point - start, v) / lenSq, Scalar(0.0), Scalar(1.0));
        return pointAt(t);
    }
    [[nodiscard]] Scalar distanceTo(const Point3& point) const noexcept { return Point3::distance(point, closestPoint(point)); }
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = 1e-10) const noexcept { return distanceTo(point) <= tolerance; }

    [[nodiscard]] std::optional<Point3> intersect(const Segment3& other, Scalar tolerance = 1e-10) const noexcept
    {
        const Vector3 d1 = vector(), d2 = other.vector(), r = start - other.start;
        const Scalar a = d1.lengthSquared(), e = d2.lengthSquared();
        if (a <= 1e-20 && e <= 1e-20) return Point3::distance(start, other.start) <= tolerance ? std::optional<Point3>{center()} : std::nullopt;
        if (a <= 1e-20) return other.contains(start, tolerance) ? std::optional<Point3>{start} : std::nullopt;
        if (e <= 1e-20) return contains(other.start, tolerance) ? std::optional<Point3>{other.start} : std::nullopt;
        const Scalar b = Vector3::dot(d1, d2), c = Vector3::dot(d1, r), f = Vector3::dot(d2, r);
        const Scalar denom = a * e - b * b;
        if (std::abs(denom) <= Scalar(1e-12) * std::max(Scalar(1.0), a * e))
        {
            if (Vector3::cross(r, d1).length() > tolerance * std::sqrt(a)) return std::nullopt;
            if (contains(other.start, tolerance)) return other.start;
            if (contains(other.end, tolerance)) return other.end;
            if (other.contains(start, tolerance)) return start;
            if (other.contains(end, tolerance)) return end;
            return std::nullopt;
        }
        const Scalar t = (b * f - c * e) / denom;
        const Scalar u = (a * f - b * c) / denom;
        if (t < -tolerance || t > 1.0 + tolerance || u < -tolerance || u > 1.0 + tolerance) return std::nullopt;
        const Point3 p1 = pointAt(std::clamp(t, Scalar(0.0), Scalar(1.0)));
        const Point3 p2 = other.pointAt(std::clamp(u, Scalar(0.0), Scalar(1.0)));
        if (Point3::distance(p1, p2) > tolerance) return std::nullopt;
        return Point3{(p1.x+p2.x)*0.5, (p1.y+p2.y)*0.5, (p1.z+p2.z)*0.5};
    }

    [[nodiscard]] static Scalar distanceBetween(const Segment3& a, const Segment3& b) noexcept
    {
        const Vector3 d1 = a.vector(), d2 = b.vector(), r = a.start - b.start;
        const Scalar aa = d1.lengthSquared(), ee = d2.lengthSquared(), eps = 1e-20;
        if (aa <= eps && ee <= eps) return Point3::distance(a.start, b.start);
        if (aa <= eps) return b.distanceTo(a.start);
        if (ee <= eps) return a.distanceTo(b.start);
        const Scalar bb = Vector3::dot(d1, d2), cc = Vector3::dot(d1, r), ff = Vector3::dot(d2, r);
        const Scalar denom = aa * ee - bb * bb;
        Scalar s = 0.0, t = 0.0;
        if (std::abs(denom) > eps)
        {
            s = std::clamp((bb * ff - cc * ee) / denom, Scalar(0.0), Scalar(1.0));
            t = (bb * s + ff) / ee;
            if (t < 0.0) { t = 0.0; s = std::clamp(-cc / aa, Scalar(0.0), Scalar(1.0)); }
            else if (t > 1.0) { t = 1.0; s = std::clamp((bb - cc) / aa, Scalar(0.0), Scalar(1.0)); }
        }
        else { t = std::clamp(ff / ee, Scalar(0.0), Scalar(1.0)); }
        return Point3::distance(a.pointAt(s), b.pointAt(t));
    }

    friend constexpr bool operator==(const Segment3& a, const Segment3& b) noexcept { return a.start == b.start && a.end == b.end; }
    friend constexpr bool operator!=(const Segment3& a, const Segment3& b) noexcept { return !(a == b); }
};

} // namespace mir
