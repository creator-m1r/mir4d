#pragma once

#include "MirEngine/BRep/Topology/BRepTypes.hpp"
#include "MirEngine/BRep/Core/BRepHandles.hpp"
#include "MirEngine/Math/Vector/Vector.hpp"
#include "MirEngine/Core/Types/Scalar.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace mir
{

struct BRepPointGeometry
{
    Vector3 point{Vector3::zero()};

    [[nodiscard]] bool isFinite() const noexcept
    {
        return point.isFinite();
    }
};

struct BRepRange
{
    Scalar first{0.0};
    Scalar last{1.0};

    [[nodiscard]] constexpr Scalar length() const noexcept { return last - first; }

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return std::isfinite(first) && std::isfinite(last) && last >= first;
    }

    [[nodiscard]] constexpr bool contains(Scalar u, Scalar tol = Scalar(0.0)) const noexcept
    {
        return u >= first - tol && u <= last + tol;
    }
};

struct BRepLineCurve
{
    Vector3 location{Vector3::zero()};
    Vector3 direction{Vector3::unitX()};

    [[nodiscard]] Vector3 value(Scalar u) const noexcept { return location + direction * u; }
    [[nodiscard]] Vector3 derivative() const noexcept { return direction; }

    [[nodiscard]] bool isValid() const noexcept
    {
        return location.isFinite() && direction.isFinite() && !direction.isZero();
    }
};

struct BRepCircleCurve
{
    Vector3 center{Vector3::zero()};
    Vector3 normal{Vector3::unitZ()};
    Vector3 xDir{Vector3::unitX()};
    Scalar radius{1.0};

    [[nodiscard]] Vector3 yDir() const noexcept
    {
        return Vector3::cross(normal, xDir).normalized();
    }

    [[nodiscard]] Vector3 value(Scalar angle) const noexcept
    {
        const Vector3 y = yDir();
        return center + xDir * (radius * std::cos(angle)) + y * (radius * std::sin(angle));
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        return center.isFinite() && normal.isFinite() && xDir.isFinite() &&
               std::isfinite(radius) && radius > Scalar(0.0) &&
               !normal.isZero() && !xDir.isZero();
    }
};

struct BRepPolylineCurve
{
    std::vector<Vector3> poles;

    [[nodiscard]] bool isValid() const noexcept
    {
        if (poles.size() < 2)
            return false;
        for (const Vector3& p : poles)
            if (!p.isFinite())
                return false;
        return true;
    }
};

struct BRepCurveGeometry
{
    BRepCurveType type{BRepCurveType::Unknown};
    BRepRange range{};
    BRepLineCurve line{};
    BRepCircleCurve circle{};
    BRepPolylineCurve polyline{};

    [[nodiscard]] bool isValid() const noexcept
    {
        if (!range.isValid())
            return false;

        switch (type)
        {
            case BRepCurveType::Line: return line.isValid();
            case BRepCurveType::Circle:
            case BRepCurveType::Arc: return circle.isValid();
            case BRepCurveType::Polyline: return polyline.isValid();
            default: return false;
        }
    }

    [[nodiscard]] Vector3 valueAt(Scalar u) const noexcept
    {
        switch (type)
        {
            case BRepCurveType::Line: return line.value(u);
            case BRepCurveType::Circle:
            case BRepCurveType::Arc: return circle.value(u);
            case BRepCurveType::Polyline:
            {
                if (polyline.poles.empty()) return Vector3::zero();
                if (polyline.poles.size() == 1) return polyline.poles.front();

                const std::size_t segCount = polyline.poles.size() - 1;
                const Scalar t = range.length() > Scalar(0.0)
                    ? (u - range.first) / range.length()
                    : Scalar(0.0);
                const Scalar clamped = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
                const Scalar scaled = clamped * static_cast<Scalar>(segCount);
                const std::size_t i = static_cast<std::size_t>(scaled);
                const std::size_t i0 = i >= segCount ? segCount - 1 : i;
                const std::size_t i1 = i0 + 1;
                const Scalar local = scaled - static_cast<Scalar>(i0);
                const Vector3& a = polyline.poles[i0];
                const Vector3& b = polyline.poles[i1];
                return a + (b - a) * local;
            }
            default: return Vector3::zero();
        }
    }
};

struct BRepPlaneSurface
{
    Vector3 location{Vector3::zero()};
    Vector3 normal{Vector3::unitZ()};
    Vector3 xDir{Vector3::unitX()};

    [[nodiscard]] Vector3 yDir() const noexcept
    {
        return Vector3::cross(normal, xDir).normalized();
    }

    [[nodiscard]] Vector3 value(Scalar u, Scalar v) const noexcept
    {
        return location + xDir * u + yDir() * v;
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        return location.isFinite() && normal.isFinite() && xDir.isFinite() &&
               !normal.isZero() && !xDir.isZero();
    }
};

struct BRepCylinderSurface
{
    Vector3 location{Vector3::zero()};
    Vector3 axis{Vector3::unitZ()};
    Vector3 xDir{Vector3::unitX()};
    Scalar radius{1.0};

    [[nodiscard]] bool isValid() const noexcept
    {
        return location.isFinite() && axis.isFinite() && xDir.isFinite() &&
               std::isfinite(radius) && radius > Scalar(0.0) &&
               !axis.isZero() && !xDir.isZero();
    }
};

struct BRepSurfaceGeometry
{
    BRepSurfaceType type{BRepSurfaceType::Unknown};
    BRepRange uRange{};
    BRepRange vRange{};
    BRepPlaneSurface plane{};
    BRepCylinderSurface cylinder{};

    [[nodiscard]] bool isValid() const noexcept
    {
        if (!uRange.isValid() || !vRange.isValid())
            return false;

        switch (type)
        {
            case BRepSurfaceType::Plane: return plane.isValid();
            case BRepSurfaceType::Cylinder: return cylinder.isValid();
            default: return false;
        }
    }

    [[nodiscard]] Vector3 valueAt(Scalar u, Scalar v) const noexcept
    {
        switch (type)
        {
            case BRepSurfaceType::Plane: return plane.value(u, v);
            case BRepSurfaceType::Cylinder:
            {
                const Vector3 yDir = Vector3::cross(cylinder.axis, cylinder.xDir).normalized();
                const Vector3 radial = cylinder.xDir * std::cos(u) + yDir * std::sin(u);
                return cylinder.location + radial * cylinder.radius + cylinder.axis * v;
            }
            default: return Vector3::zero();
        }
    }
};

}
