// MirEngine/Math/Point.hpp
// 📍 Каноническая математическая точка 3D.
//
// Point3 + Vector3 = Point3
// Point3 - Vector3 = Point3
// Point3 - Point3  = Vector3
//
// C++23

#pragma once

#include "../Core/Types/Scalar.hpp"
#include "Vector/Vector.hpp"

#include <cmath>
#include <compare>
#include <cstddef>
#include <functional>
#include <ostream>

namespace mir::math
{

class Point3
{
public:
    using ScalarType = mir::Scalar;

    Scalar x{0.0};
    Scalar y{0.0};
    Scalar z{0.0};

    constexpr Point3() noexcept = default;
    constexpr Point3(Scalar x_, Scalar y_, Scalar z_) noexcept : x(x_), y(y_), z(z_) {}

    [[nodiscard]] static constexpr Point3 origin() noexcept { return {0.0, 0.0, 0.0}; }

    [[nodiscard]] constexpr Point3 operator+(const mir::Vector3& v) const noexcept { return {x + v.x, y + v.y, z + v.z}; }
    [[nodiscard]] constexpr Point3 operator-(const mir::Vector3& v) const noexcept { return {x - v.x, y - v.y, z - v.z}; }
    [[nodiscard]] constexpr mir::Vector3 operator-(const Point3& other) const noexcept { return {x - other.x, y - other.y, z - other.z}; }

    constexpr Point3& operator+=(const mir::Vector3& v) noexcept { x += v.x; y += v.y; z += v.z; return *this; }
    constexpr Point3& operator-=(const mir::Vector3& v) noexcept { x -= v.x; y -= v.y; z -= v.z; return *this; }
    constexpr Point3& translate(const mir::Vector3& delta) noexcept { return *this += delta; }

    [[nodiscard]] constexpr Scalar squaredDistance(const Point3& other) const noexcept { return (*this - other).lengthSquared(); }
    [[nodiscard]] Scalar distance(const Point3& other) const noexcept { return std::sqrt(squaredDistance(other)); }

    [[nodiscard]] static constexpr Scalar distanceSquared(const Point3& a, const Point3& b) noexcept { return (a - b).lengthSquared(); }
    [[nodiscard]] static Scalar distance(const Point3& a, const Point3& b) noexcept { return std::sqrt(distanceSquared(a, b)); }

    [[nodiscard]] static constexpr Point3 lerp(const Point3& a, const Point3& b, Scalar t) noexcept
    {
        return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t};
    }

    [[nodiscard]] constexpr Scalar& operator[](std::size_t index) noexcept { return index == 0 ? x : index == 1 ? y : z; }
    [[nodiscard]] constexpr const Scalar& operator[](std::size_t index) const noexcept { return index == 0 ? x : index == 1 ? y : z; }

    [[nodiscard]] bool isFinite() const noexcept { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }

    friend constexpr bool operator==(const Point3& a, const Point3& b) noexcept = default;
    friend constexpr auto operator<=>(const Point3& a, const Point3& b) noexcept = default;
};

} // namespace mir::math

namespace mir
{
/// Canonical geometry-facing name; the implementation lives in mir::math::Point3.
using Point3 = math::Point3;
}

namespace std
{
template<>
struct hash<mir::math::Point3>
{
    std::size_t operator()(const mir::math::Point3& p) const noexcept
    {
        const std::size_t h1 = std::hash<mir::Scalar>{}(p.x);
        const std::size_t h2 = std::hash<mir::Scalar>{}(p.y);
        const std::size_t h3 = std::hash<mir::Scalar>{}(p.z);
        return h1 ^ (h2 + static_cast<std::size_t>(0x9e3779b9) + (h1 << 6) + (h1 >> 2)) ^
               (h3 + static_cast<std::size_t>(0x9e3779b9) + (h2 << 6) + (h2 >> 2));
    }
};
} // namespace std

inline std::ostream& operator<<(std::ostream& stream, const mir::math::Point3& point)
{
    return stream << '(' << point.x << ", " << point.y << ", " << point.z << ')';
}
