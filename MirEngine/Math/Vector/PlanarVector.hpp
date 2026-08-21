
#pragma once

#include "../../Core/Types/Scalar.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace mir
{

struct Vector2
{

    Scalar x{0.0};
    Scalar y{0.0};

    constexpr Vector2() noexcept = default;

    constexpr Vector2(
        Scalar x_,
        Scalar y_) noexcept
        : x(x_)
        , y(y_)
    {
    }

    [[nodiscard]]
    static constexpr Vector2 zero() noexcept
    {
        return {0.0, 0.0};
    }

    [[nodiscard]]
    static constexpr Vector2 one() noexcept
    {
        return {1.0, 1.0};
    }

    [[nodiscard]]
    static constexpr Vector2 unitX() noexcept
    {
        return {1.0, 0.0};
    }

    [[nodiscard]]
    static constexpr Vector2 unitY() noexcept
    {
        return {0.0, 1.0};
    }

    [[nodiscard]]
    constexpr Scalar lengthSquared() const noexcept
    {
        return x * x + y * y;
    }

    [[nodiscard]]
    Scalar length() const noexcept
    {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]]
    Vector2 normalized(
        Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        const Scalar len = length();

        if (len <= epsilon)
        {
            return zero();
        }

        return *this / len;
    }

    bool normalize(
        Scalar epsilon = Scalar(1e-20)) noexcept
    {
        const Scalar len = length();

        if (len <= epsilon)
        {
            return false;
        }

        *this /= len;
        return true;
    }

    [[nodiscard]]
    static constexpr Scalar dot(
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return a.x * b.x + a.y * b.y;
    }

    [[nodiscard]]
    constexpr Scalar dot(
        const Vector2& other) const noexcept
    {
        return dot(*this, other);
    }

    [[nodiscard]]
    static constexpr Scalar cross(
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return a.x * b.y - a.y * b.x;
    }

    [[nodiscard]]
    constexpr Scalar cross(
        const Vector2& other) const noexcept
    {
        return cross(*this, other);
    }

    [[nodiscard]]
    static constexpr Scalar distanceSquared(
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return (a - b).lengthSquared();
    }

    [[nodiscard]]
    static Scalar distance(
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return std::sqrt(
            distanceSquared(a, b)
        );
    }

    [[nodiscard]]
    Scalar angleTo(
        const Vector2& other,
        Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        const Scalar denominator =
            std::sqrt(
                lengthSquared() *
                other.lengthSquared()
            );

        if (denominator <= epsilon)
        {
            return Scalar(0.0);
        }

        const Scalar cosine =
            std::clamp(
                dot(other) / denominator,
                Scalar(-1.0),
                Scalar(1.0)
            );

        return std::acos(cosine);
    }

    [[nodiscard]]
    static constexpr Vector2 lerp(
        const Vector2& a,
        const Vector2& b,
        Scalar t) noexcept
    {
        return
        {
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        };
    }

    [[nodiscard]]
    static constexpr Vector2 componentMin(
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return
        {
            std::min(a.x, b.x),
            std::min(a.y, b.y)
        };
    }

    [[nodiscard]]
    static constexpr Vector2 componentMax(
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return
        {
            std::max(a.x, b.x),
            std::max(a.y, b.y)
        };
    }

    [[nodiscard]]
    static Vector2 componentAbs(
        const Vector2& value) noexcept
    {
        return
        {
            std::abs(value.x),
            std::abs(value.y)
        };
    }

    friend constexpr Vector2 operator+(
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return {a.x + b.x, a.y + b.y};
    }

    friend constexpr Vector2 operator- (
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return {a.x - b.x, a.y - b.y};
    }

    friend constexpr Vector2 operator* (
        const Vector2& value,
        Scalar scalar) noexcept
    {
        return
        {
            value.x * scalar,
            value.y * scalar
        };
    }

    friend constexpr Vector2 operator* (
        Scalar scalar,
        const Vector2& value) noexcept
    {
        return value * scalar;
    }

    friend constexpr Vector2 operator/ (
        const Vector2& value,
        Scalar scalar) noexcept
    {
        return
        {
            value.x / scalar,
            value.y / scalar
        };
    }

    friend constexpr Vector2 operator- (
        const Vector2& value) noexcept
    {
        return {-value.x, -value.y};
    }

    constexpr Vector2& operator+=(
        const Vector2& other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vector2& operator-=(
        const Vector2& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vector2& operator*=(
        Scalar scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vector2& operator/=(
        Scalar scalar) noexcept
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    [[nodiscard]]
    constexpr Scalar& operator[](
        std::size_t index) noexcept
    {
        return index == 0 ? x : y;
    }

    [[nodiscard]]
    constexpr const Scalar& operator[](
        std::size_t index) const noexcept
    {
        return index == 0 ? x : y;
    }

    [[nodiscard]]
    bool isZero(
        Scalar epsilon = Scalar(1e-12)) const noexcept
    {
        return
            std::abs(x) <= epsilon &&
            std::abs(y) <= epsilon;
    }

    [[nodiscard]]
    bool isFinite() const noexcept
    {
        return
            std::isfinite(x) &&
            std::isfinite(y);
    }

    friend constexpr bool operator==(
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return a.x == b.x && a.y == b.y;
    }

    friend constexpr bool operator!=(
        const Vector2& a,
        const Vector2& b) noexcept
    {
        return !(a == b);
    }
};

}
