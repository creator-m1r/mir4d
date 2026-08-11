// MirEngine/Math/Precision.hpp
// 🎯 Глобальные настройки точности для всех геометрических вычислений MirEngine.
//
// В инженерных расчётах критически важно понимать, с какой точностью
// мы работаем. Нельзя просто сравнивать два числа типа double на равенство —
// из-за погрешностей вычислений 1.0 может превратиться в 0.9999999999.
// Precision централизует все допуски в одном месте, чтобы любой алгоритм
// мог сказать: «считаем эти точки совпадающими с точностью до микрона»
// или «этот угол равен нулю с точностью до нанорадиана».
//
// Что здесь хранится:
//   • linearTolerance  — допуск для линейных величин (расстояний, координат).
//                         По умолчанию 1e-10 метра = 0.1 нанометра. В САПР
//                         часто используют 1e-6 (микрон) или 1e-9 (нанометр).
//   • angularTolerance — допуск для угловых величин (радиан). По умолчанию
//                         1e-12 радиан ≈ 5.7e-11 градуса.
//   • areEqual(a,b)    — проверяет, равны ли два числа с учётом linearTolerance.
//   • isZero(v)        — проверяет, близко ли число к нулю.
//   • areEqualAngles   — проверяет, равны ли два угла с учётом angularTolerance.
//
// Это не класс для создания экземпляров — все методы статические.
// При желании можно создать экземпляр с другими значениями допусков
// (например, для грубых расчётов или сверхточных).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include <cmath>                       // std::abs

namespace mir {

class Precision {
public:
    // ── Допуски (можно менять под задачу) ────────────────────
    Scalar linearTolerance  = Scalar(1e-10);   // 0.1 нм
    Scalar angularTolerance = Scalar(1e-12);   // ~5.7e-11 градуса

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Precision() noexcept = default;

    constexpr Precision(Scalar linear, Scalar angular) noexcept
        : linearTolerance(linear), angularTolerance(angular)
    {}

    // ── Сравнение чисел ──────────────────────────────────────

    // Два числа равны с учётом линейного допуска?
    [[nodiscard]] bool areEqual(Scalar a, Scalar b) const noexcept {
        return std::abs(a - b) <= linearTolerance;
    }

    // Число практически ноль?
    [[nodiscard]] bool isZero(Scalar value) const noexcept {
        return std::abs(value) <= linearTolerance;
    }

    // Два угла равны с учётом углового допуска?
    [[nodiscard]] bool areEqualAngles(Scalar a, Scalar b) const noexcept {
        return std::abs(a - b) <= angularTolerance;
    }

    // Угол практически ноль?
    [[nodiscard]] bool isZeroAngle(Scalar angle) const noexcept {
        return std::abs(angle) <= angularTolerance;
    }

    // ── Статические методы (используют глобальный экземпляр) ──
    // Удобно, когда не нужен свой Precision.

    static bool approximatelyEqual(Scalar a, Scalar b) noexcept {
        static Precision defaultPrecision;
        return defaultPrecision.areEqual(a, b);
    }

    static bool approximatelyZero(Scalar value) noexcept {
        static Precision defaultPrecision;
        return defaultPrecision.isZero(value);
    }

    // ── Глобальный экземпляр (синглтон) ──────────────────────
    // Можно настроить один раз при старте движка, и все алгоритмы
    // будут использовать эти значения.
    static Precision& global() noexcept {
        static Precision s_global;
        return s_global;
    }
};

} // namespace mir