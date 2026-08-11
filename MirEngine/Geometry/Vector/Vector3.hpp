#pragma once

#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace mir
{

/**
 * @brief Трёхмерный математический вектор.
 *
 * Vector3 используется для:
 *  - направлений;
 *  - скоростей;
 *  - нормалей;
 *  - масштабов;
 *  - координатных преобразований;
 *  - векторных операций геометрического ядра.
 *
 * Важно:
 * Point3 и Vector3 являются разными сущностями.
 * Vector3 не должен неявно превращаться в Point3.
 */
struct Vector3
{
    using Scalar = double;

    Scalar x{0.0};
    Scalar y{0.0};
    Scalar z{0.0};

    // ------------------------------------------------------------
    // Конструкторы
    // ------------------------------------------------------------

    constexpr Vector3() noexcept = default;

    constexpr Vector3(Scalar xValue,
                      Scalar yValue,
                      Scalar zValue) noexcept
        : x(xValue),
          y(yValue),
          z(zValue)
    {
    }

    // ------------------------------------------------------------
    // Фабричные методы
    // ------------------------------------------------------------

    [[nodiscard]]
    static constexpr Vector3 zero() noexcept
    {
        return Vector3{0.0, 0.0, 0.0};
    }

    [[nodiscard]]
    static constexpr Vector3 unitX() noexcept
    {
        return Vector3{1.0, 0.0, 0.0};
    }

    [[nodiscard]]
    static constexpr Vector3 unitY() noexcept
    {
        return Vector3{0.0, 1.0, 0.0};
    }

    [[nodiscard]]
    static constexpr Vector3 unitZ() noexcept
    {
        return Vector3{0.0, 0.0, 1.0};
    }

    // ------------------------------------------------------------
    // Индексация
    // ------------------------------------------------------------

    [[nodiscard]]
    constexpr Scalar& operator[](std::size_t index) noexcept
    {
        switch (index)
        {
            case 0:
                return x;

            case 1:
                return y;

            default:
                return z;
        }
    }

    [[nodiscard]]
    constexpr const Scalar& operator[](std::size_t index) const noexcept
    {
        switch (index)
        {
            case 0:
                return x;

            case 1:
                return y;

            default:
                return z;
        }
    }

    // ------------------------------------------------------------
    // Арифметика
    // ------------------------------------------------------------

    [[nodiscard]]
    constexpr Vector3 operator+(const Vector3& other) const noexcept
    {
        return Vector3{
            x + other.x,
            y + other.y,
            z + other.z
        };
    }

    [[nodiscard]]
    constexpr Vector3 operator-(const Vector3& other) const noexcept
    {
        return Vector3{
            x - other.x,
            y - other.y,
            z - other.z
        };
    }

    [[nodiscard]]
    constexpr Vector3 operator-() const noexcept
    {
        return Vector3{
            -x,
            -y,
            -z
        };
    }

    [[nodiscard]]
    constexpr Vector3 operator*(Scalar value) const noexcept
    {
        return Vector3{
            x * value,
            y * value,
            z * value
        };
    }

    [[nodiscard]]
    constexpr Vector3 operator/(Scalar value) const noexcept
    {
        return Vector3{
            x / value,
            y / value,
            z / value
        };
    }

    // ------------------------------------------------------------
    // Составные операции
    // ------------------------------------------------------------

    constexpr Vector3& operator+=(const Vector3& other) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;

        return *this;
    }

    constexpr Vector3& operator-=(const Vector3& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;

        return *this;
    }

    constexpr Vector3& operator*=(Scalar value) noexcept
    {
        x *= value;
        y *= value;
        z *= value;

        return *this;
    }

    constexpr Vector3& operator/=(Scalar value) noexcept
    {
        x /= value;
        y /= value;
        z /= value;

        return *this;
    }

    // ------------------------------------------------------------
    // Скалярные характеристики
    // ------------------------------------------------------------

    [[nodiscard]]
    constexpr Scalar squaredLength() const noexcept
    {
        return x * x + y * y + z * z;
    }

    [[nodiscard]]
    Scalar length() const noexcept
    {
        return std::sqrt(squaredLength());
    }

    [[nodiscard]]
    constexpr Scalar dot(const Vector3& other) const noexcept
    {
        return x * other.x
             + y * other.y
             + z * other.z;
    }

    // ------------------------------------------------------------
    // Векторное произведение
    // ------------------------------------------------------------

    [[nodiscard]]
    constexpr Vector3 cross(const Vector3& other) const noexcept
    {
        return Vector3{
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    // ------------------------------------------------------------
    // Нормализация
    // ------------------------------------------------------------

    [[nodiscard]]
    Vector3 normalized() const noexcept
    {
        const Scalar len = length();

        if (len <= std::numeric_limits<Scalar>::epsilon())
        {
            return Vector3::zero();
        }

        return *this / len;
    }

    void normalize() noexcept
    {
        const Scalar len = length();

        if (len <= std::numeric_limits<Scalar>::epsilon())
        {
            x = 0.0;
            y = 0.0;
            z = 0.0;
            return;
        }

        x /= len;
        y /= len;
        z /= len;
    }

    // ------------------------------------------------------------
    // Проверки
    // ------------------------------------------------------------

    [[nodiscard]]
    bool isZero(
        Scalar epsilon =
            std::numeric_limits<Scalar>::epsilon() * 100.0) const noexcept
    {
        return squaredLength() <= epsilon * epsilon;
    }

    [[nodiscard]]
    bool isFinite() const noexcept
    {
        return std::isfinite(x)
            && std::isfinite(y)
            && std::isfinite(z);
    }

    [[nodiscard]]
    bool isNearlyEqual(
        const Vector3& other,
        Scalar epsilon = 1e-9) const noexcept
    {
        return std::abs(x - other.x) <= epsilon
            && std::abs(y - other.y) <= epsilon
            && std::abs(z - other.z) <= epsilon;
    }

    // ------------------------------------------------------------
    // Геометрия
    // ------------------------------------------------------------

    [[nodiscard]]
    Scalar distanceTo(const Vector3& other) const noexcept
    {
        return (*this - other).length();
    }

    [[nodiscard]]
    constexpr Scalar squaredDistanceTo(
        const Vector3& other) const noexcept
    {
        return (*this - other).squaredLength();
    }

    [[nodiscard]]
    Scalar angleTo(const Vector3& other) const noexcept
    {
        const Scalar lhsLength = length();
        const Scalar rhsLength = other.length();

        const Scalar denominator = lhsLength * rhsLength;

        if (denominator <= std::numeric_limits<Scalar>::epsilon())
        {
            return 0.0;
        }

        Scalar cosine = dot(other) / denominator;

        // Защита от погрешности вычислений.
        if (cosine > 1.0)
        {
            cosine = 1.0;
        }
        else if (cosine < -1.0)
        {
            cosine = -1.0;
        }

        return std::acos(cosine);
    }

    // ------------------------------------------------------------
    // Линейная интерполяция
    // ------------------------------------------------------------

    [[nodiscard]]
    constexpr Vector3 lerp(
        const Vector3& other,
        Scalar t) const noexcept
    {
        return Vector3{
            x + (other.x - x) * t,
            y + (other.y - y) * t,
            z + (other.z - z) * t
        };
    }

    // ------------------------------------------------------------
    // Скалярное тройное произведение
    // ------------------------------------------------------------

    [[nodiscard]]
    constexpr Scalar tripleProduct(
        const Vector3& b,
        const Vector3& c) const noexcept
    {
        return dot(b.cross(c));
    }
};


// ============================================================================
// Свободные операторы
// ============================================================================

/**
 * @brief Умножение скаляра на вектор.
 *
 * ВАЖНО:
 * Здесь оставлена только операция Scalar * Vector3.
 *
 * Операция Vector3 * Scalar уже реализована
 * как метод Vector3::operator*().
 *
 * Это устраняет конфликт IntelliSense,
 * который возникал из-за дублирования двух одинаково
 * подходящих operator*.
 */
[[nodiscard]]
constexpr Vector3 operator*(
    Vector3::Scalar value,
    const Vector3& vector) noexcept
{
    return Vector3{
        vector.x * value,
        vector.y * value,
        vector.z * value
    };
}


// ============================================================================
// Сравнение
// ============================================================================

[[nodiscard]]
constexpr bool operator==(
    const Vector3& a,
    const Vector3& b) noexcept
{
    return a.x == b.x
        && a.y == b.y
        && a.z == b.z;
}

[[nodiscard]]
constexpr bool operator!=(
    const Vector3& a,
    const Vector3& b) noexcept
{
    return !(a == b);
}


// ============================================================================
// Свободные математические функции
// ============================================================================

[[nodiscard]]
constexpr Vector3 cross(
    const Vector3& a,
    const Vector3& b) noexcept
{
    return Vector3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}


[[nodiscard]]
constexpr Vector3 lerp(
    const Vector3& a,
    const Vector3& b,
    Vector3::Scalar t) noexcept
{
    return Vector3{
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    };
}


[[nodiscard]]
constexpr Vector3 min(
    const Vector3& a,
    const Vector3& b) noexcept
{
    return Vector3{
        a.x < b.x ? a.x : b.x,
        a.y < b.y ? a.y : b.y,
        a.z < b.z ? a.z : b.z
    };
}


[[nodiscard]]
constexpr Vector3 max(
    const Vector3& a,
    const Vector3& b) noexcept
{
    return Vector3{
        a.x > b.x ? a.x : b.x,
        a.y > b.y ? a.y : b.y,
        a.z > b.z ? a.z : b.z
    };
}

} // namespace mir