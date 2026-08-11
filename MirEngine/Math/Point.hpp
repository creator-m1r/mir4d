// MirEngine/Math/Point.hpp
// 📍 Математическая точка — базовый тип для представления положения в пространстве.
//
// Point3 (трёхмерная точка) — это фундаментальный математический объект,
// который описывает положение в трёхмерном пространстве тремя координатами
// (x, y, z). В отличие от вектора (Vector3), который обозначает направление
// или смещение, точка — это конкретное место. Сложение двух точек не имеет
// математического смысла (нельзя сложить "Москва" + "Париж"), но разность
// двух точек даёт вектор (смещение от одной к другой).
//
// Этот класс находится в Math, а не в Geometry, потому что точка —
// это абстрактное математическое понятие, не привязанное к конкретной
// геометрической сущности (как Point3 в Geometry, который может быть
// частью линии, плоскости и т.д.). Math::Point3 используется везде:
// в векторах, матрицах, кватернионах, интерполяции.
//
// Чистый C++23, без внешних зависимостей.


#pragma once

#include "../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include "Vector/Vector3.hpp"         // mir::Vector3
#include <compare>                    // для оператора <=>
#include <functional>                 // для std::hash
#include <ostream>                    // для вывода в поток

namespace mir::math {

class Point3 {
public:
    mir::Scalar x = 0.0;
    mir::Scalar y = 0.0;
    mir::Scalar z = 0.0;

    constexpr Point3() noexcept = default;
    constexpr Point3(mir::Scalar x, mir::Scalar y, mir::Scalar z) noexcept : x(x), y(y), z(z) {}

    [[nodiscard]] static constexpr Point3 origin() noexcept { return {0.0, 0.0, 0.0}; }

    friend constexpr Point3 operator+(const Point3& p, const mir::Vector3& v) noexcept {
        return {p.x + v.x, p.y + v.y, p.z + v.z};
    }
    friend constexpr Point3 operator-(const Point3& p, const mir::Vector3& v) noexcept {
        return {p.x - v.x, p.y - v.y, p.z - v.z};
    }

    friend constexpr mir::Vector3 operator-(const Point3& a, const Point3& b) noexcept {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    constexpr Point3& operator+=(const mir::Vector3& v) noexcept { x += v.x; y += v.y; z += v.z; return *this; }
    constexpr Point3& operator-=(const mir::Vector3& v) noexcept { x -= v.x; y -= v.y; z -= v.z; return *this; }

    friend constexpr bool operator==(const Point3& a, const Point3& b) noexcept = default;
    friend constexpr auto operator<=>(const Point3& a, const Point3& b) noexcept = default;

    [[nodiscard]] static mir::Scalar distance(const Point3& a, const Point3& b) noexcept {
        return (a - b).length();
    }
    [[nodiscard]] static constexpr mir::Scalar distanceSquared(const Point3& a, const Point3& b) noexcept {
        return (a - b).lengthSquared();
    }

    [[nodiscard]] static constexpr Point3 lerp(const Point3& a, const Point3& b, mir::Scalar t) noexcept {
        return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t};
    }

    [[nodiscard]] constexpr mir::Scalar& operator[](int i) noexcept { return (&x)[i]; }
    [[nodiscard]] constexpr const mir::Scalar& operator[](int i) const noexcept { return (&x)[i]; }
};

} // namespace mir::math

// ── Хеш-функция ──────────────────────────────────────────────
namespace std {
template <>
struct hash<mir::math::Point3> {
    [[nodiscard]] std::size_t operator()(const mir::math::Point3& p) const noexcept {
        std::size_t h1 = std::hash<mir::Scalar>{}(p.x);
        std::size_t h2 = std::hash<mir::Scalar>{}(p.y);
        std::size_t h3 = std::hash<mir::Scalar>{}(p.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
} // namespace std

// ── Вывод в поток ────────────────────────────────────────────
inline std::ostream& operator<<(std::ostream& os, const mir::math::Point3& p) {
    return os << '(' << p.x << ", " << p.y << ", " << p.z << ')';
}