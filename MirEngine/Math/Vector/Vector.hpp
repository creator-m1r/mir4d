
#pragma once

#include "../../Core/Types/Scalar.hpp"

#include <algorithm>
#include <cmath>
#include <compare>
#include <cstddef>
#include <functional>
#include <ostream>
#include <tuple>
#include <utility>

namespace mir
{

struct Vector3
{

    Scalar x{0.0};
    Scalar y{0.0};
    Scalar z{0.0};

    constexpr Vector3() noexcept = default;

    constexpr Vector3(
        Scalar x_,
        Scalar y_,
        Scalar z_) noexcept
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }

    [[nodiscard]]
    static constexpr Vector3 zero() noexcept
    {
        return {0.0, 0.0, 0.0};
    }

    [[nodiscard]]
    static constexpr Vector3 one() noexcept
    {
        return {1.0, 1.0, 1.0};
    }

    [[nodiscard]]
    static constexpr Vector3 unitX() noexcept
    {
        return {1.0, 0.0, 0.0};
    }

    [[nodiscard]]
    static constexpr Vector3 unitY() noexcept
    {
        return {0.0, 1.0, 0.0};
    }

    [[nodiscard]]
    static constexpr Vector3 unitZ() noexcept
    {
        return {0.0, 0.0, 1.0};
    }

    [[nodiscard]]
    constexpr Scalar lengthSquared() const noexcept
    {
        return
            x * x +
            y * y +
            z * z;
    }

    [[nodiscard]]
    Scalar length() const noexcept
    {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]]
    Vector3 normalized(
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
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return
            a.x * b.x +
            a.y * b.y +
            a.z * b.z;
    }

    [[nodiscard]]
    constexpr Scalar dot(
        const Vector3& other) const noexcept
    {
        return dot(*this, other);
    }

    [[nodiscard]]
    static constexpr Vector3 cross(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return
        {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    [[nodiscard]]
    constexpr Vector3 cross(
        const Vector3& other) const noexcept
    {
        return cross(*this, other);
    }

    [[nodiscard]]
    static constexpr Scalar distanceSquared(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return (a - b).lengthSquared();
    }

    [[nodiscard]]
    static Scalar distance(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return std::sqrt(
            distanceSquared(a, b)
        );
    }

    [[nodiscard]]
    Scalar angleTo(
        const Vector3& other,
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
    Vector3 projectedOnto(
        const Vector3& other,
        Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        const Scalar lengthSq =
            other.lengthSquared();

        if (lengthSq <= epsilon)
        {
            return zero();
        }

        return
            other *
            (dot(other) / lengthSq);
    }

    [[nodiscard]]
    Vector3 reflected(
        const Vector3& normal) const noexcept
    {
        return
            *this -
            normal *
            (Scalar(2.0) * dot(normal));
    }

    [[nodiscard]]
    static constexpr Vector3 lerp(
        const Vector3& a,
        const Vector3& b,
        Scalar t) noexcept
    {
        return
        {
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t
        };
    }

    [[nodiscard]]
    static constexpr Vector3 componentMin(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return
        {
            std::min(a.x, b.x),
            std::min(a.y, b.y),
            std::min(a.z, b.z)
        };
    }

    [[nodiscard]]
    static constexpr Vector3 componentMax(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return
        {
            std::max(a.x, b.x),
            std::max(a.y, b.y),
            std::max(a.z, b.z)
        };
    }

    [[nodiscard]]
    static Vector3 componentAbs(
        const Vector3& value) noexcept
    {
        return
        {
            std::abs(value.x),
            std::abs(value.y),
            std::abs(value.z)
        };
    }

    [[nodiscard]]
    static constexpr Vector3 componentMul(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return
        {
            a.x * b.x,
            a.y * b.y,
            a.z * b.z
        };
    }

    [[nodiscard]]
    static constexpr Vector3 componentDiv(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return
        {
            a.x / b.x,
            a.y / b.y,
            a.z / b.z
        };
    }

    friend constexpr Vector3 operator+(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return
        {
            a.x + b.x,
            a.y + b.y,
            a.z + b.z
        };
    }

    friend constexpr Vector3 operator-(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return
        {
            a.x - b.x,
            a.y - b.y,
            a.z - b.z
        };
    }

    friend constexpr Vector3 operator*(
        const Vector3& v,
        Scalar value) noexcept
    {
        return
        {
            v.x * value,
            v.y * value,
            v.z * value
        };
    }

    friend constexpr Vector3 operator*(
        Scalar value,
        const Vector3& v) noexcept
    {
        return v * value;
    }

    friend constexpr Vector3 operator/(
        const Vector3& v,
        Scalar value) noexcept
    {
        return
        {
            v.x / value,
            v.y / value,
            v.z / value
        };
    }

    friend constexpr Vector3 operator-(
        const Vector3& v) noexcept
    {
        return
        {
            -v.x,
            -v.y,
            -v.z
        };
    }

    constexpr Vector3& operator+=(
        const Vector3& other) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;

        return *this;
    }

    constexpr Vector3& operator-=(
        const Vector3& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;

        return *this;
    }

    constexpr Vector3& operator*=(
        Scalar value) noexcept
    {
        x *= value;
        y *= value;
        z *= value;

        return *this;
    }

    constexpr Vector3& operator/=(
        Scalar value) noexcept
    {
        x /= value;
        y /= value;
        z /= value;

        return *this;
    }

    [[nodiscard]]
    constexpr Scalar& operator[](
        std::size_t index) noexcept
    {
        return
            index == 0 ? x :
            index == 1 ? y :
            z;
    }

    [[nodiscard]]
    constexpr const Scalar& operator[](
        std::size_t index) const noexcept
    {
        return
            index == 0 ? x :
            index == 1 ? y :
            z;
    }

    [[nodiscard]]
    bool isZero(
        Scalar epsilon = Scalar(1e-12)) const noexcept
    {
        return
            std::abs(x) <= epsilon &&
            std::abs(y) <= epsilon &&
            std::abs(z) <= epsilon;
    }

    [[nodiscard]]
    bool isFinite() const noexcept
    {
        return
            std::isfinite(x) &&
            std::isfinite(y) &&
            std::isfinite(z);
    }

    friend constexpr bool operator==(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return
            a.x == b.x &&
            a.y == b.y &&
            a.z == b.z;
    }

    friend constexpr bool operator!=(
        const Vector3& a,
        const Vector3& b) noexcept
    {
        return !(a == b);
    }

    friend constexpr auto operator<=>(
        const Vector3& a,
        const Vector3& b) noexcept = default;
};

template <std::size_t I>
[[nodiscard]]
constexpr Scalar& get(
    Vector3& value) noexcept
{
    static_assert(
        I < 3,
        "Vector3 index must be 0, 1 or 2"
    );

    if constexpr (I == 0)
    {
        return value.x;
    }
    else if constexpr (I == 1)
    {
        return value.y;
    }
    else
    {
        return value.z;
    }
}

template <std::size_t I>
[[nodiscard]]
constexpr const Scalar& get(
    const Vector3& value) noexcept
{
    static_assert(
        I < 3,
        "Vector3 index must be 0, 1 or 2"
    );

    if constexpr (I == 0)
    {
        return value.x;
    }
    else if constexpr (I == 1)
    {
        return value.y;
    }
    else
    {
        return value.z;
    }
}

template <std::size_t I>
[[nodiscard]]
constexpr Scalar&& get(
    Vector3&& value) noexcept
{
    static_assert(
        I < 3,
        "Vector3 index must be 0, 1 or 2"
    );

    if constexpr (I == 0)
    {
        return std::move(value.x);
    }
    else if constexpr (I == 1)
    {
        return std::move(value.y);
    }
    else
    {
        return std::move(value.z);
    }
}

template <std::size_t I>
[[nodiscard]]
constexpr const Scalar&& get(
    const Vector3&& value) noexcept
{
    static_assert(
        I < 3,
        "Vector3 index must be 0, 1 or 2"
    );

    if constexpr (I == 0)
    {
        return std::move(value.x);
    }
    else if constexpr (I == 1)
    {
        return std::move(value.y);
    }
    else
    {
        return std::move(value.z);
    }
}

}

namespace std
{

template <>
struct tuple_size<mir::Vector3>
    : std::integral_constant<std::size_t, 3>
{
};

template <std::size_t I>
struct tuple_element<I, mir::Vector3>
{
    static_assert(
        I < 3,
        "Vector3 index must be 0, 1 or 2"
    );

    using type = mir::Scalar;
};

template <>
struct hash<mir::Vector3>
{
    [[nodiscard]]
    std::size_t operator()(
        const mir::Vector3& value) const noexcept
    {
        const std::size_t hx =
            std::hash<mir::Scalar>{}(value.x);

        const std::size_t hy =
            std::hash<mir::Scalar>{}(value.y);

        const std::size_t hz =
            std::hash<mir::Scalar>{}(value.z);

        constexpr std::size_t magic =
            static_cast<std::size_t>(
                0x9e3779b97f4a7c15ULL
            );

        std::size_t result = hx;

        result ^=
            hy +
            magic +
            (result << 6) +
            (result >> 2);

        result ^=
            hz +
            magic +
            (result << 6) +
            (result >> 2);

        return result;
    }
};

}

inline std::ostream& operator<<(
    std::ostream& stream,
    const mir::Vector3& value)
{
    return
        stream
        << '['
        << value.x
        << ", "
        << value.y
        << ", "
        << value.z
        << ']';
}