// MirEngine/Math/Vector/Vector4.hpp
// 🧭 Четырёхмерный вектор — для однородных координат и матричных преобразований.
//
// Vector4 похож на Vector3, но имеет дополнительную координату w.
// Эта четвёртая компонента делает возможным представление 3D-точек
// и направлений в однородных координатах, что критически важно
// для работы с матрицами 4×4 (Matrix4).
//
// Простыми словами:
//   • Если w = 1.0 — это точка в пространстве.
//   • Если w = 0.0 — это направление (вектор), на который не влияет перенос.
// Именно так работают все современные графические движки (OpenGL, Metal, DirectX).
//
// Vector4 используется для:
//   • Умножения на Matrix4 (трансформация точек и векторов).
//   • Хранения цветов (RGBA), где w — альфа-канал (прозрачность).
//   • Передачи данных в графический конвейер (шейдеры).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include <cmath>                          // sqrt

namespace mir {

struct Vector4 {
    // ── Компоненты ───────────────────────────────────────────
    Scalar x = 0.0;   // координата X
    Scalar y = 0.0;   // координата Y
    Scalar z = 0.0;   // координата Z
    Scalar w = 0.0;   // однородная компонента (0 = направление, 1 = точка)

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Vector4() noexcept = default;

    constexpr Vector4(Scalar x, Scalar y, Scalar z, Scalar w) noexcept
        : x(x), y(y), z(z), w(w)
    {}

    // Создать Vector4 из Vector3 + w (удобно для точек и направлений).
    explicit Vector4(const Vector3& v, Scalar w = 1.0) noexcept
        : x(v.x), y(v.y), z(v.z), w(w)
    {}

    // ── Статические константы ────────────────────────────────
    [[nodiscard]] static constexpr Vector4 zero() noexcept {
        return {0.0, 0.0, 0.0, 0.0};
    }

    // ── Геометрические операции ──────────────────────────────

    // Квадрат длины (только x,y,z — w не учитывается).
    [[nodiscard]] constexpr Scalar lengthSquared() const noexcept {
        return x * x + y * y + z * z;
    }

    // Длина вектора (без учёта w).
    [[nodiscard]] Scalar length() const noexcept {
        return std::sqrt(lengthSquared());
    }

    // Нормализация (приводит x,y,z к единичной длине, w не меняется).
    [[nodiscard]] Vector4 normalized() const noexcept {
        Scalar len = length();
        if (len < 1e-20) {
            return zero();
        }
        return {x / len, y / len, z / len, w};
    }

    // ── Скалярное произведение (по x,y,z) ────────────────────
    [[nodiscard]] static constexpr Scalar dot(const Vector4& a, const Vector4& b) noexcept {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    // ── Арифметические операторы ─────────────────────────────
    friend constexpr Vector4 operator+(const Vector4& a, const Vector4& b) noexcept {
        return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    }
    friend constexpr Vector4 operator-(const Vector4& a, const Vector4& b) noexcept {
        return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    }
    friend constexpr Vector4 operator*(const Vector4& v, Scalar s) noexcept {
        return {v.x * s, v.y * s, v.z * s, v.w * s};
    }
    friend constexpr Vector4 operator*(Scalar s, const Vector4& v) noexcept {
        return {v.x * s, v.y * s, v.z * s, v.w * s};
    }
    friend constexpr Vector4 operator/(const Vector4& v, Scalar s) noexcept {
        return {v.x / s, v.y / s, v.z / s, v.w / s};
    }

    constexpr Vector4 operator-() const noexcept {
        return {-x, -y, -z, -w};
    }

    Vector4& operator+=(const Vector4& other) noexcept {
        x += other.x; y += other.y; z += other.z; w += other.w;
        return *this;
    }
    Vector4& operator-=(const Vector4& other) noexcept {
        x -= other.x; y -= other.y; z -= other.z; w -= other.w;
        return *this;
    }
    Vector4& operator*=(Scalar s) noexcept {
        x *= s; y *= s; z *= s; w *= s;
        return *this;
    }
    Vector4& operator/=(Scalar s) noexcept {
        x /= s; y /= s; z /= s; w /= s;
        return *this;
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Vector4& a, const Vector4& b) noexcept {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }
    friend constexpr bool operator!=(const Vector4& a, const Vector4& b) noexcept {
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