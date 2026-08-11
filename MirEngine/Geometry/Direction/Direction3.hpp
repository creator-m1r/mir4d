// MirEngine/Geometry/Direction/Direction3.hpp
// 🧭 Направление в трёхмерном пространстве — единичный вектор с гарантией.
//
// Direction3 — это особый вид вектора, который всегда имеет длину ровно 1.
// Обычный Vector3 может иметь любую длину, а Direction3 — нет. Это очень
// полезно, когда ты работаешь с направлениями: нормали к поверхностям,
// ориентация камеры, направление света, ось вращения. Везде, где нужен
// "чистый" вектор без масштаба.
//
// Почему это важно:
//   • Компилятор и документация сразу говорят: "это направление, оно единичное".
//   • Не нужно каждый раз проверять длину — Direction3 гарантирует её при создании.
//   • Направления можно сравнивать, складывать (получая вектор), умножать на скаляр
//     (превращая в обычный вектор нужной длины).
//
// Как создать Direction3:
//   • Direction3 dir = Direction3::fromVector(v);   // из любого вектора (авто-нормализация)
//   • Direction3 dir = Direction3::unitX();         // готовые константы
//   • Direction3 dir = Direction3::fromAngles(azimuth, elevation); // из сферических углов
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cmath>
#include <compare>
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include "../../Math/Vector/Vector3.hpp"  // mir::Vector3
#include "../../Core/Types/Angle.hpp"     // mir::Angle

namespace mir {

class Direction3 {
public:
    // ── Компоненты (гарантированно единичная длина) ──────────
    Scalar x = 1.0;   // компонента X (направление по оси X)
    Scalar y = 0.0;   // компонента Y
    Scalar z = 0.0;   // компонента Z

    // ── Конструкторы (приватные для безопасности) ─────────────
    // Мы не даём публичного конструктора от трёх чисел — чтобы нельзя было
    // случайно создать "направление" с длиной ≠ 1.0. Вместо этого используй
    // статические фабрики.

    constexpr Direction3() noexcept = default;

    // ── Статические фабрики ──────────────────────────────────

    // Создать направление из вектора. Вектор будет нормализован.
    // Если вектор нулевой, возвращается направление unitX (защита от деления на 0).
    [[nodiscard]] static Direction3 fromVector(const Vector3& v) noexcept {
        Scalar len = v.length();
        if (len < Scalar(1e-20)) {
            return unitX();   // нулевой вектор — fallback на ось X
        }
        return Direction3(v.x / len, v.y / len, v.z / len);
    }

    // Готовые направления вдоль осей координат.
    [[nodiscard]] static constexpr Direction3 unitX() noexcept { return Direction3(1.0, 0.0, 0.0); }
    [[nodiscard]] static constexpr Direction3 unitY() noexcept { return Direction3(0.0, 1.0, 0.0); }
    [[nodiscard]] static constexpr Direction3 unitZ() noexcept { return Direction3(0.0, 0.0, 1.0); }
    [[nodiscard]] static constexpr Direction3 negativeUnitX() noexcept { return Direction3(-1.0, 0.0, 0.0); }
    [[nodiscard]] static constexpr Direction3 negativeUnitY() noexcept { return Direction3(0.0, -1.0, 0.0); }
    [[nodiscard]] static constexpr Direction3 negativeUnitZ() noexcept { return Direction3(0.0, 0.0, -1.0); }

    // Создать направление из сферических углов (азимут и угол места).
    // azimuth   — угол в горизонтальной плоскости (от оси X, против часовой стрелки, если смотреть сверху).
    // elevation — угол над горизонтом (от плоскости XY вверх к Z).
    [[nodiscard]] static Direction3 fromSpherical(const Angle& azimuth, const Angle& elevation) noexcept {
        Scalar cosElev = std::cos(elevation.radians());
        Scalar sinElev = std::sin(elevation.radians());
        Scalar cosAz   = std::cos(azimuth.radians());
        Scalar sinAz   = std::sin(azimuth.radians());
        return Direction3(cosElev * cosAz, cosElev * sinAz, sinElev);
    }

    // ── Преобразование в Vector3 ─────────────────────────────
    // Получить вектор длины 1 (тот же, что хранится внутри).
    [[nodiscard]] constexpr Vector3 asVector() const noexcept {
        return {x, y, z};
    }

    // Получить вектор нужной длины, указывающий в этом направлении.
    [[nodiscard]] constexpr Vector3 scaled(Scalar length) const noexcept {
        return {x * length, y * length, z * length};
    }

    // ── Инверсия направления ─────────────────────────────────
    [[nodiscard]] constexpr Direction3 opposite() const noexcept {
        return Direction3(-x, -y, -z);
    }
    constexpr Direction3 operator-() const noexcept {
        return opposite();
    }

    // ── Сравнение направлений ────────────────────────────────
    // Два направления равны, если их компоненты равны.
    friend constexpr bool operator==(const Direction3& a, const Direction3& b) noexcept = default;

    // ── Угол между направлениями ─────────────────────────────
    [[nodiscard]] Scalar angleTo(const Direction3& other) const noexcept {
        Scalar dot = x * other.x + y * other.y + z * other.z;
        dot = std::clamp(dot, Scalar(-1.0), Scalar(1.0));
        return std::acos(dot);
    }

    [[nodiscard]] Scalar angleToDegrees(const Direction3& other) const noexcept {
        return angleTo(other) * Scalar(180.0 / 3.14159265358979323846);
    }

    // ── Удобные операции ─────────────────────────────────────
    [[nodiscard]] constexpr Scalar dot(const Vector3& v) const noexcept {
        return x * v.x + y * v.y + z * v.z;
    }
    [[nodiscard]] constexpr Scalar dot(const Direction3& d) const noexcept {
        return x * d.x + y * d.y + z * d.z;
    }
    [[nodiscard]] constexpr Vector3 cross(const Vector3& v) const noexcept {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }

private:
    // Приватный конструктор — только для внутреннего использования.
    // Принимает уже нормализованные компоненты.
    constexpr Direction3(Scalar nx, Scalar ny, Scalar nz) noexcept
        : x(nx), y(ny), z(nz)
    {
        // В отладочной сборке можно добавить assert, что длина ≈ 1.0.
        // В релизе — доверяем вызывающему коду.
    }
};

} // namespace mir