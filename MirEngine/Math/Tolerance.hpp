// MirEngine/Math/Tolerance.hpp
// ⚖️ Допуски для сравнения вещественных чисел в инженерных расчётах.
//
// В компьютерной графике и CAD числа с плавающей точкой (double/float)
// никогда не бывают абсолютно точными. Например, после серии вычислений
// может получиться 0.0000000001 вместо ожидаемого 0.0. Если сравнивать
// такие числа напрямую (a == b), результат будет непредсказуемым.
//
// Tolerance решает эту проблему: вместо точного равенства мы проверяем,
// достаточно ли числа близки друг к другу. "Достаточно" определяется
// двумя параметрами:
//   • linearTolerance  — допустимая погрешность для расстояний (мм).
//   • angularTolerance — допустимая погрешность для углов (радианы).
//
// Пример:
//   Tolerance tol{1e-6, 1e-9};     // 0.001 микрона для длины, нанорадиан для угла
//   tol.isZero(0.000000001);        // true — почти ноль
//   tol.areEqual(1.0, 1.0000000001); // true — почти одинаковы
//
// В будущем каждый Document или Project сможет иметь свои настройки
// точности, но глобальный Tolerance даёт разумные значения по умолчанию.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include "Vector/Vector3.hpp"             // mir::Vector3
#include <cmath>                          // std::abs

namespace mir {

class Tolerance {
public:
    // ── Значения по умолчанию ────────────────────────────────
    // 1e-10 мм — достаточно для большинства инженерных задач.
    // 1e-12 рад — примерно 0.000000000057 градуса.
    Scalar linearTolerance  = 1e-10;
    Scalar angularTolerance = 1e-12;

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Tolerance() noexcept = default;

    constexpr Tolerance(Scalar linear, Scalar angular) noexcept
        : linearTolerance(linear), angularTolerance(angular)
    {}

    // ── Сравнение чисел ──────────────────────────────────────

    // Два числа равны с учётом допуска?
    [[nodiscard]] bool areEqual(Scalar a, Scalar b) const noexcept {
        return std::abs(a - b) <= linearTolerance;
    }

    // Число практически ноль?
    [[nodiscard]] bool isZero(Scalar value) const noexcept {
        return std::abs(value) <= linearTolerance;
    }

    // ── Сравнение векторов ───────────────────────────────────
    // Два вектора равны покомпонентно с учётом допуска?
    [[nodiscard]] bool areEqual(const Vector3& a, const Vector3& b) const noexcept {
        return areEqual(a.x, b.x) && areEqual(a.y, b.y) && areEqual(a.z, b.z);
    }

    // Вектор практически нулевой (все компоненты близки к 0)?
    [[nodiscard]] bool isZero(const Vector3& v) const noexcept {
        return isZero(v.x) && isZero(v.y) && isZero(v.z);
    }

    // ── Сравнение углов ──────────────────────────────────────
    // Два угла равны с учётом углового допуска?
    [[nodiscard]] bool areEqualAngles(Scalar a, Scalar b) const noexcept {
        return std::abs(a - b) <= angularTolerance;
    }

    // Угол практически ноль?
    [[nodiscard]] bool isZeroAngle(Scalar angle) const noexcept {
        return std::abs(angle) <= angularTolerance;
    }

    // ── Статические утилиты (используют дефолтный допуск) ────
    // Удобные функции, когда не нужен свой экземпляр Tolerance.

    static bool approximatelyEqual(Scalar a, Scalar b) noexcept {
        static Tolerance defaultTol;
        return defaultTol.areEqual(a, b);
    }

    static bool approximatelyZero(Scalar value) noexcept {
        static Tolerance defaultTol;
        return defaultTol.isZero(value);
    }
};

} // namespace mir