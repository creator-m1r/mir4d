// MirEngine/Geometry/Direction/Direction.hpp
// 🧭 Единичное направление 3D.
#pragma once

#include <algorithm>
#include <cmath>
#include <compare>

#include "../../Core/Types/Scalar.hpp"
#include "../../Core/Types/Angle.hpp"
#include "../../Math/Vector/Vector.hpp"

namespace mir
{

class Direction3
{
public:
    Scalar x{1.0};
    Scalar y{0.0};
    Scalar z{0.0};

    constexpr Direction3() noexcept = default;

    [[nodiscard]] static Direction3 fromVector(const Vector3& value) noexcept
    {
        const Scalar length = value.length();
        if (length <= Scalar(1e-20)) return unitX();
        return Direction3(value.x / length, value.y / length, value.z / length);
    }

    [[nodiscard]] static constexpr Direction3 unitX() noexcept { return {1.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Direction3 unitY() noexcept { return {0.0, 1.0, 0.0}; }
    [[nodiscard]] static constexpr Direction3 unitZ() noexcept { return {0.0, 0.0, 1.0}; }
    [[nodiscard]] static constexpr Direction3 negativeUnitX() noexcept { return {-1.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Direction3 negativeUnitY() noexcept { return {0.0, -1.0, 0.0}; }
    [[nodiscard]] static constexpr Direction3 negativeUnitZ() noexcept { return {0.0, 0.0, -1.0}; }

    [[nodiscard]] static Direction3 fromSpherical(const Angle& azimuth, const Angle& elevation) noexcept
    {
        const Scalar ec = std::cos(elevation.radians());
        const Scalar es = std::sin(elevation.radians());
        const Scalar ac = std::cos(azimuth.radians());
        const Scalar as = std::sin(azimuth.radians());
        return fromVector({ec * ac, ec * as, es});
    }

    [[nodiscard]] constexpr Vector3 asVector() const noexcept { return {x, y, z}; }
    [[nodiscard]] constexpr Vector3 scaled(Scalar length) const noexcept { return {x * length, y * length, z * length}; }
    [[nodiscard]] constexpr Direction3 opposite() const noexcept { return Direction3(-x, -y, -z); }
    [[nodiscard]] constexpr Direction3 operator-() const noexcept { return opposite(); }

    [[nodiscard]] Scalar angleTo(const Direction3& other) const noexcept
    {
        const Scalar value = std::clamp(x * other.x + y * other.y + z * other.z, Scalar(-1.0), Scalar(1.0));
        return std::acos(value);
    }
    [[nodiscard]] Scalar angleToDegrees(const Direction3& other) const noexcept
    { return angleTo(other) * Scalar(180.0 / 3.14159265358979323846); }

    [[nodiscard]] constexpr Scalar dot(const Vector3& value) const noexcept { return x * value.x + y * value.y + z * value.z; }
    [[nodiscard]] constexpr Scalar dot(const Direction3& value) const noexcept { return x * value.x + y * value.y + z * value.z; }
    [[nodiscard]] constexpr Vector3 cross(const Vector3& value) const noexcept
    { return {y * value.z - z * value.y, z * value.x - x * value.z, x * value.y - y * value.x}; }
    [[nodiscard]] constexpr bool isFinite() const noexcept { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }

    friend constexpr bool operator==(const Direction3& a, const Direction3& b) noexcept = default;

private:
    constexpr Direction3(Scalar nx, Scalar ny, Scalar nz) noexcept : x(nx), y(ny), z(nz) {}
};

} // namespace mir
