// MirEngine/Core/Types/Angle.hpp
// 📐 Типобезопасный угол — всегда хранится в радианах, но понимает градусы.
//
// В геометрии и инженерных расчётах углы могут измеряться в радианах
// (математически удобно) или в градусах (привычно человеку). Очень часто
// в коде передают просто число (double), и невозможно понять, радианы это
// или градусы. В результате возникают трудноуловимые ошибки: поворот на
// 90 радиан вместо 90 градусов, и всё "едет".
//
// Класс Angle решает эту проблему навсегда:
//   • Внутри ВСЕГДА хранит значение в радианах (единая система для расчётов).
//   • Снаружи можно создать угол из радиан (Angle::radians) или градусов
//     (Angle::degrees) — явно, без путаницы.
//   • Получить значение можно только через radians() или degrees() —
//     сразу видно, в каких единицах ты берёшь число.
//   • Нельзя случайно присвоить Angle обычному double и наоборот.
//
// Примеры:
//   Angle a = Angle::degrees(90.0);      // создаём угол 90°
//   Scalar r = a.radians();              // получаем ≈1.5708 рад
//   Angle b = Angle::radians(3.14159);   // создаём угол из радиан
//   double d = b.degrees();              // получаем ≈180°
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Scalar.hpp"   // наш основной числовой тип (double)
#include <numbers>      // std::numbers::pi для C++20/23
#include <cmath>        // std::fmod, std::abs

namespace mir {

class Angle {
public:
    // ── Фабричные методы (единственный способ создать Angle) ──
    // Мы НЕ даём публичного конструктора от числа — чтобы нельзя было
    // случайно создать Angle из "голого" double, не указав единицы.

    // Создать угол из радиан.
    [[nodiscard]] static constexpr Angle radians(Scalar value) noexcept {
        return Angle(value, true);   // true = "это радианы"
    }

    // Создать угол из градусов.
    [[nodiscard]] static constexpr Angle degrees(Scalar value) noexcept {
        // Переводим градусы в радианы: π/180
        constexpr Scalar DEG_TO_RAD = 3.14159265358979323846 / 180.0;
        return Angle(value * DEG_TO_RAD, true);
    }

    // ── Получить значение в нужных единицах ──────────────────

    // Значение в радианах.
    [[nodiscard]] constexpr Scalar radians() const noexcept {
        return m_radians;
    }

    // Значение в градусах.
    [[nodiscard]] constexpr Scalar degrees() const noexcept {
        constexpr Scalar RAD_TO_DEG = 180.0 / 3.14159265358979323846;
        return m_radians * RAD_TO_DEG;
    }

    // ── Часто используемые константы ─────────────────────────
    [[nodiscard]] static constexpr Angle zero() noexcept {
        return radians(0.0);
    }
    [[nodiscard]] static constexpr Angle halfPi() noexcept {
        return radians(1.5707963267948966);  // π/2
    }
    [[nodiscard]] static constexpr Angle pi() noexcept {
        return radians(3.1415926535897932);  // π
    }
    [[nodiscard]] static constexpr Angle twoPi() noexcept {
        return radians(6.2831853071795865);  // 2π
    }

    // ── Арифметические операции ──────────────────────────────
    friend constexpr Angle operator+(Angle a, Angle b) noexcept {
        return radians(a.m_radians + b.m_radians);
    }
    friend constexpr Angle operator-(Angle a, Angle b) noexcept {
        return radians(a.m_radians - b.m_radians);
    }
    friend constexpr Angle operator*(Angle a, Scalar s) noexcept {
        return radians(a.m_radians * s);
    }
    friend constexpr Angle operator/(Angle a, Scalar s) noexcept {
        return radians(a.m_radians / s);
    }
    friend constexpr Scalar operator/(Angle a, Angle b) noexcept {
        return a.m_radians / b.m_radians;   // отношение углов — безразмерное число
    }

    // Унарный минус.
    constexpr Angle operator-() const noexcept {
        return radians(-m_radians);
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(Angle a, Angle b) noexcept {
        return a.m_radians == b.m_radians;
    }
    friend constexpr bool operator!=(Angle a, Angle b) noexcept {
        return a.m_radians != b.m_radians;
    }

    // ── Нормализация ─────────────────────────────────────────
    // Приводит угол к диапазону [0, 2π).
    [[nodiscard]] Angle normalized() const noexcept {
        Scalar twoPi = 2.0 * 3.14159265358979323846;
        Scalar normalized = std::fmod(m_radians, twoPi);
        if (normalized < 0.0) normalized += twoPi;
        return radians(normalized);
    }

private:
    // Приватный конструктор. Второй параметр — просто тэг, чтобы
    // отличать от "голого" числа; он не используется, но гарантирует,
    // что никто снаружи не вызовет конструктор без явного указания единиц.
    constexpr Angle(Scalar radians, bool /*isRadians*/) noexcept
        : m_radians(radians)
    {}

    Scalar m_radians = 0.0;   // всегда в радианах
};

} // namespace mir