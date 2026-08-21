#pragma once

#include "../../Core/Types/Scalar.hpp"
#include "Vector.hpp"

#include <cmath>
#include <cstddef>

namespace mir
{

struct Vector4
{
    Scalar x{0.0};
    Scalar y{0.0};
    Scalar z{0.0};
    Scalar w{0.0};

    constexpr Vector4() noexcept = default;
    constexpr Vector4(Scalar x_, Scalar y_, Scalar z_, Scalar w_) noexcept
        : x(x_), y(y_), z(z_), w(w_) {}
    constexpr explicit Vector4(const Vector3& value, Scalar w_ = Scalar(1.0)) noexcept
        : x(value.x), y(value.y), z(value.z), w(w_) {}

    [[nodiscard]] static constexpr Vector4 zero() noexcept { return {0.0, 0.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vector4 unitX() noexcept { return {1.0, 0.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vector4 unitY() noexcept { return {0.0, 1.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vector4 unitZ() noexcept { return {0.0, 0.0, 1.0, 0.0}; }
    [[nodiscard]] static constexpr Vector4 unitW() noexcept { return {0.0, 0.0, 0.0, 1.0}; }

    [[nodiscard]] constexpr Scalar lengthSquared() const noexcept { return x * x + y * y + z * z; }
    [[nodiscard]] Scalar length() const noexcept { return std::sqrt(lengthSquared()); }

    [[nodiscard]] Vector4 normalized(Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        const Scalar len = length();
        if (len <= epsilon) return {0.0, 0.0, 0.0, w};
        return {x / len, y / len, z / len, w};
    }

    bool normalize(Scalar epsilon = Scalar(1e-20)) noexcept
    {
        const Scalar len = length();
        if (len <= epsilon) return false;
        x /= len; y /= len; z /= len;
        return true;
    }

    [[nodiscard]] static constexpr Scalar dot(const Vector4& a, const Vector4& b) noexcept
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    [[nodiscard]] constexpr Scalar dot(const Vector4& other) const noexcept { return dot(*this, other); }

    friend constexpr Vector4 operator+(const Vector4& a, const Vector4& b) noexcept
    { return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}; }
    friend constexpr Vector4 operator-(const Vector4& a, const Vector4& b) noexcept
    { return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}; }
    friend constexpr Vector4 operator*(const Vector4& value, Scalar scalar) noexcept
    { return {value.x * scalar, value.y * scalar, value.z * scalar, value.w * scalar}; }
    friend constexpr Vector4 operator*(Scalar scalar, const Vector4& value) noexcept
    { return value * scalar; }
    friend constexpr Vector4 operator/(const Vector4& value, Scalar scalar) noexcept
    { return {value.x / scalar, value.y / scalar, value.z / scalar, value.w / scalar}; }
    friend constexpr Vector4 operator-(const Vector4& value) noexcept
    { return {-value.x, -value.y, -value.z, -value.w}; }

    constexpr Vector4& operator+=(const Vector4& other) noexcept
    { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
    constexpr Vector4& operator-=(const Vector4& other) noexcept
    { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
    constexpr Vector4& operator*=(Scalar scalar) noexcept
    { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
    constexpr Vector4& operator/=(Scalar scalar) noexcept
    { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

    [[nodiscard]] constexpr Vector3 xyz() const noexcept { return {x, y, z}; }

    [[nodiscard]] constexpr Scalar& operator[](std::size_t index) noexcept
    { return index == 0 ? x : index == 1 ? y : index == 2 ? z : w; }
    [[nodiscard]] constexpr const Scalar& operator[](std::size_t index) const noexcept
    { return index == 0 ? x : index == 1 ? y : index == 2 ? z : w; }

    [[nodiscard]] bool isFinite() const noexcept
    { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z) && std::isfinite(w); }

    friend constexpr bool operator==(const Vector4& a, const Vector4& b) noexcept
    { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
    friend constexpr bool operator!=(const Vector4& a, const Vector4& b) noexcept
    { return !(a == b); }
};

} // namespace mir
