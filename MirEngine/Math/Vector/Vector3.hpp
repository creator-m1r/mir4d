// MirEngine/Math/Vector/Vector3.hpp
// 🧭 Трёхмерный вектор — основа всей геометрии MirEngine.
//
// Vector3 представляет точку или направление в трёхмерном пространстве.
// Он используется везде: координаты вершин, нормали, скорости, силы,
// направления лучей, позиции объектов и камер.
//
// Почему это так важно:
//   • ВСЕ геометрические расчёты в движке строятся на Vector3.
//   • Это простой, но мощный тип — как кирпичик, из которого
//     складывается всё здание геометрии.
//   • Благодаря перегрузке операторов (+, -, *, /) с векторами
//     можно работать почти как с обычными числами.
//
// Возможности:
//   • Сложение и вычитание векторов.
//   • Умножение на число (масштабирование).
//   • Длина вектора (length) и квадрат длины (lengthSquared).
//   • Нормализация (превращение в вектор единичной длины).
//   • Скалярное произведение (dot) — мера "сонаправленности".
//   • Векторное произведение (cross) — даёт перпендикулярный вектор.
//   • Расстояние между точками (distance).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <compare>
#include <functional>
#include <ostream>
#include <tuple>
#include <cmath>
#include <algorithm>

#include "../../Core/Types/Scalar.hpp"   // mir::Scalar = double

namespace mir {

struct Vector3 {
    // ── Компоненты ───────────────────────────────────────────
    Scalar x = 0.0;   // координата по оси X (вперёд/назад)
    Scalar y = 0.0;   // координата по оси Y (вверх/вниз)
    Scalar z = 0.0;   // координата по оси Z (вправо/влево)

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Vector3() noexcept = default;
    constexpr Vector3(Scalar x, Scalar y, Scalar z) noexcept : x(x), y(y), z(z) {}

    // ── Статические константы ────────────────────────────────
    [[nodiscard]] static constexpr Vector3 zero() noexcept  { return {0.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vector3 unitX() noexcept { return {1.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vector3 unitY() noexcept { return {0.0, 1.0, 0.0}; }
    [[nodiscard]] static constexpr Vector3 unitZ() noexcept { return {0.0, 0.0, 1.0}; }
    [[nodiscard]] static constexpr Vector3 one() noexcept   { return {1.0, 1.0, 1.0}; }

    // ── Геометрические операции ──────────────────────────────
    [[nodiscard]] constexpr Scalar lengthSquared() const noexcept {
        return x * x + y * y + z * z;
    }
    [[nodiscard]] Scalar length() const noexcept {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]] Vector3 normalized() const noexcept {
        Scalar len = length();
        return (len > Scalar(1e-20)) ? Vector3{x / len, y / len, z / len} : zero();
    }
    void normalize() noexcept {
        Scalar len = length();
        if (len > Scalar(1e-20)) {
            x /= len; y /= len; z /= len;
        }
    }

    // ── Скалярное и векторное произведения ───────────────────
    [[nodiscard]] static constexpr Scalar dot(const Vector3& a, const Vector3& b) noexcept {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    [[nodiscard]] static constexpr Vector3 cross(const Vector3& a, const Vector3& b) noexcept {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    // ── Проекция и отражение ─────────────────────────────────
    [[nodiscard]] Vector3 projectedOnto(const Vector3& other) const noexcept {
        Scalar lenSq = other.lengthSquared();
        return (lenSq > Scalar(1e-20)) ? other * (dot(*this, other) / lenSq) : zero();
    }
    [[nodiscard]] Vector3 reflected(const Vector3& normal) const noexcept {
        return *this - normal * (Scalar(2) * dot(*this, normal));
    }

    // ── Расстояние ───────────────────────────────────────────
    [[nodiscard]] static Scalar distance(const Vector3& a, const Vector3& b) noexcept {
        return (a - b).length();
    }
    [[nodiscard]] static constexpr Scalar distanceSquared(const Vector3& a, const Vector3& b) noexcept {
        return (a - b).lengthSquared();
    }

    // ── Угол между векторами ─────────────────────────────────
    [[nodiscard]] Scalar angleTo(const Vector3& other) const noexcept {
        Scalar denom = std::sqrt(lengthSquared() * other.lengthSquared());
        if (denom < Scalar(1e-20)) return Scalar(0);
        Scalar cosVal = std::clamp(dot(*this, other) / denom, Scalar(-1), Scalar(1));
        return std::acos(cosVal);
    }

    // ── Линейная интерполяция ────────────────────────────────
    [[nodiscard]] static constexpr Vector3 lerp(const Vector3& a, const Vector3& b, Scalar t) noexcept {
        return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t};
    }

    // ── Покомпонентные операции ──────────────────────────────
    [[nodiscard]] static constexpr Vector3 componentMin(const Vector3& a, const Vector3& b) noexcept {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
    }
    [[nodiscard]] static constexpr Vector3 componentMax(const Vector3& a, const Vector3& b) noexcept {
        return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
    }
    [[nodiscard]] static constexpr Vector3 componentAbs(const Vector3& v) noexcept {
        return {std::abs(v.x), std::abs(v.y), std::abs(v.z)};
    }

    // ── Арифметические операторы ─────────────────────────────
    friend constexpr Vector3 operator+(const Vector3& a, const Vector3& b) noexcept { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
    friend constexpr Vector3 operator-(const Vector3& a, const Vector3& b) noexcept { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
    friend constexpr Vector3 operator*(const Vector3& v, Scalar s) noexcept { return {v.x * s, v.y * s, v.z * s}; }
    friend constexpr Vector3 operator*(Scalar s, const Vector3& v) noexcept { return {v.x * s, v.y * s, v.z * s}; }
    friend constexpr Vector3 operator/(const Vector3& v, Scalar s) noexcept { return {v.x / s, v.y / s, v.z / s}; }
    constexpr Vector3 operator-() const noexcept { return {-x, -y, -z}; }

    Vector3& operator+=(const Vector3& other) noexcept { x += other.x; y += other.y; z += other.z; return *this; }
    Vector3& operator-=(const Vector3& other) noexcept { x -= other.x; y -= other.y; z -= other.z; return *this; }
    Vector3& operator*=(Scalar s) noexcept { x *= s; y *= s; z *= s; return *this; }
    Vector3& operator/=(Scalar s) noexcept { x /= s; y /= s; z /= s; return *this; }

    // Покомпонентное умножение/деление
    [[nodiscard]] static constexpr Vector3 componentMul(const Vector3& a, const Vector3& b) noexcept { return {a.x * b.x, a.y * b.y, a.z * b.z}; }
    [[nodiscard]] static constexpr Vector3 componentDiv(const Vector3& a, const Vector3& b) noexcept { return {a.x / b.x, a.y / b.y, a.z / b.z}; }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Vector3& a, const Vector3& b) noexcept = default;
    friend constexpr auto operator<=>(const Vector3& a, const Vector3& b) noexcept = default;

    // ── Доступ по индексу ────────────────────────────────────
    [[nodiscard]] constexpr Scalar& operator[](int i) noexcept { return (&x)[i]; }
    [[nodiscard]] constexpr const Scalar& operator[](int i) const noexcept { return (&x)[i]; }

    // ── Проверки ─────────────────────────────────────────────
    [[nodiscard]] constexpr bool isZero() const noexcept { return *this == zero(); }
};
} // namespace mir

// ── Хеш-функция ──────────────────────────────────────────────
namespace std {
    template <>
    struct hash<mir::Vector3> {
        [[nodiscard]] size_t operator()(const mir::Vector3& v) const noexcept {
            size_t h1 = hash<mir::Scalar>{}(v.x);
            size_t h2 = hash<mir::Scalar>{}(v.y);
            size_t h3 = hash<mir::Scalar>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

// ── Вывод в поток ────────────────────────────────────────────
inline std::ostream& operator<<(std::ostream& os, const mir::Vector3& v) {
    return os << '[' << v.x << ", " << v.y << ", " << v.z << ']';
}

// ── Структурные привязки (structured bindings) ────────────────
template <> struct std::tuple_size<mir::Vector3> : std::integral_constant<std::size_t, 3> {};
template <std::size_t I> struct std::tuple_element<I, mir::Vector3> { using type = mir::Scalar; };

template <std::size_t I> [[nodiscard]] constexpr mir::Scalar& get(mir::Vector3& v) noexcept {
    if constexpr (I == 0) return v.x;
    else if constexpr (I == 1) return v.y;
    else if constexpr (I == 2) return v.z;
}
template <std::size_t I> [[nodiscard]] constexpr const mir::Scalar& get(const mir::Vector3& v) noexcept {
    if constexpr (I == 0) return v.x;
    else if constexpr (I == 1) return v.y;
    else if constexpr (I == 2) return v.z;
}
template <std::size_t I> [[nodiscard]] constexpr mir::Scalar&& get(mir::Vector3&& v) noexcept {
    if constexpr (I == 0) return std::move(v.x);
    else if constexpr (I == 1) return std::move(v.y);
    else if constexpr (I == 2) return std::move(v.z);
}