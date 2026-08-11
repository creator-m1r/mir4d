// MirEngine/Math/Vector/Vector2.hpp
// 🧭 Двумерный вектор — точка или направление на плоскости.
//
// Vector2 — это младший брат Vector3. Он работает точно так же,
// но имеет только две координаты: x и y. Используется для:
//   • 2D-эскизов (Sketch) — основа параметрического моделирования.
//   • UV-координат текстур.
//   • Расчётов на плоскости (например, профиль для выдавливания).
//   • Элементов интерфейса (позиции кнопок, размеры панелей).
//
// Все методы аналогичны Vector3, но без оси Z.
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include <cmath>                          // sqrt

namespace mir {

struct Vector2 {
    // ── Компоненты ───────────────────────────────────────────
    Scalar x = 0.0;   // горизонтальная координата
    Scalar y = 0.0;   // вертикальная координата

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Vector2() noexcept = default;

    constexpr Vector2(Scalar x, Scalar y) noexcept
        : x(x), y(y)
    {}

    // ── Статические константы ────────────────────────────────
    [[nodiscard]] static constexpr Vector2 zero() noexcept {
        return {0.0, 0.0};
    }
    [[nodiscard]] static constexpr Vector2 unitX() noexcept {
        return {1.0, 0.0};
    }
    [[nodiscard]] static constexpr Vector2 unitY() noexcept {
        return {0.0, 1.0};
    }

    // ── Геометрические операции ──────────────────────────────

    // Квадрат длины (быстрее, чем length()).
    [[nodiscard]] constexpr Scalar lengthSquared() const noexcept {
        return x * x + y * y;
    }

    // Длина вектора.
    [[nodiscard]] Scalar length() const noexcept {
        return std::sqrt(lengthSquared());
    }

    // Вектор единичной длины в том же направлении.
    [[nodiscard]] Vector2 normalized() const noexcept {
        Scalar len = length();
        if (len < 1e-20) {
            return zero();
        }
        return {x / len, y / len};
    }

    // ── Скалярное произведение ───────────────────────────────
    [[nodiscard]] static constexpr Scalar dot(const Vector2& a, const Vector2& b) noexcept {
        return a.x * b.x + a.y * b.y;
    }

    // ── Расстояние между точками ─────────────────────────────
    [[nodiscard]] static Scalar distance(const Vector2& a, const Vector2& b) noexcept {
        return (a - b).length();
    }

    [[nodiscard]] static constexpr Scalar distanceSquared(const Vector2& a, const Vector2& b) noexcept {
        return (a - b).lengthSquared();
    }

    // ── Арифметические операторы ─────────────────────────────
    friend constexpr Vector2 operator+(const Vector2& a, const Vector2& b) noexcept {
        return {a.x + b.x, a.y + b.y};
    }
    friend constexpr Vector2 operator-(const Vector2& a, const Vector2& b) noexcept {
        return {a.x - b.x, a.y - b.y};
    }
    friend constexpr Vector2 operator*(const Vector2& v, Scalar s) noexcept {
        return {v.x * s, v.y * s};
    }
    friend constexpr Vector2 operator*(Scalar s, const Vector2& v) noexcept {
        return {v.x * s, v.y * s};
    }
    friend constexpr Vector2 operator/(const Vector2& v, Scalar s) noexcept {
        return {v.x / s, v.y / s};
    }

    constexpr Vector2 operator-() const noexcept {
        return {-x, -y};
    }

    Vector2& operator+=(const Vector2& other) noexcept {
        x += other.x; y += other.y;
        return *this;
    }
    Vector2& operator-=(const Vector2& other) noexcept {
        x -= other.x; y -= other.y;
        return *this;
    }
    Vector2& operator*=(Scalar s) noexcept {
        x *= s; y *= s;
        return *this;
    }
    Vector2& operator/=(Scalar s) noexcept {
        x /= s; y /= s;
        return *this;
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Vector2& a, const Vector2& b) noexcept {
        return a.x == b.x && a.y == b.y;
    }
    friend constexpr bool operator!=(const Vector2& a, const Vector2& b) noexcept {
        return !(a == b);
    }

    // ── Доступ по индексу ────────────────────────────────────
    [[nodiscard]] constexpr Scalar& operator[](int i) noexcept {
        return (&x)[i];
    }
    [[nodiscard]] constexpr const Scalar& operator[](int i) const noexcept {
        return (&x)[i];
    }
};

} // namespace mir