// MirEngine/Math/MathUtils.hpp
// 🧮 Общие математические утилиты, константы и вспомогательные функции,
//    используемые во всех модулях MirEngine.
//
// Этот заголовок содержит часто используемые математические операции,
// которые не привязаны к конкретному классу (Vector3, Angle и т.д.).
// Здесь живут:
//   • Константы (π, e, золотое сечение…).
//   • Функции для работы с углами (градусы ↔ радианы).
//   • Линейная интерполяция (lerp), гладкая интерполяция (smoothstep).
//   • Зажим значений (clamp), сравнение с допуском (approxEqual).
//   • Преобразования между различными представлениями.
//
// Всё это — кирпичики, из которых строятся более сложные алгоритмы
// в геометрии, физике и рендеринге. Каждая функция максимально проста
// и не зависит от других модулей.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cmath>                       // std::sin, std::cos, std::sqrt, std::abs, std::fmod
#include <algorithm>                   // std::clamp, std::min, std::max
#include <numbers>                     // std::numbers::pi (C++20/23)
#include <type_traits>                 // std::is_floating_point_v, std::is_arithmetic_v

namespace mir::math {

// ═══════════════════════════════════════════════════════════════
//  Константы
// ═══════════════════════════════════════════════════════════════

/// Число π (отношение длины окружности к диаметру).
inline constexpr double PI = 3.14159265358979323846;

/// Удвоенное π (полный оборот в радианах).
inline constexpr double TWO_PI = 2.0 * PI;

/// Половина π.
inline constexpr double HALF_PI = PI / 2.0;

/// Число Эйлера (основание натурального логарифма).
inline constexpr double E = 2.71828182845904523536;

/// Золотое сечение φ = (1 + √5) / 2.
inline constexpr double GOLDEN_RATIO = 1.61803398874989484820;

/// Коэффициент для перевода градусов в радианы: π / 180.
inline constexpr double DEG_TO_RAD = PI / 180.0;

/// Коэффициент для перевода радиан в градусы: 180 / π.
inline constexpr double RAD_TO_DEG = 180.0 / PI;

// ═══════════════════════════════════════════════════════════════
//  Углы
// ═══════════════════════════════════════════════════════════════

/// Переводит градусы в радианы.
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T degreesToRadians(T degrees) noexcept {
    return degrees * static_cast<T>(DEG_TO_RAD);
}

/// Переводит радианы в градусы.
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T radiansToDegrees(T radians) noexcept {
    return radians * static_cast<T>(RAD_TO_DEG);
}

// ═══════════════════════════════════════════════════════════════
//  Интерполяция
// ═══════════════════════════════════════════════════════════════

/// Линейная интерполяция между a и b.
/// t = 0 → a, t = 1 → b.
template <typename T, typename S>
    requires std::is_arithmetic_v<T> && std::is_arithmetic_v<S>
[[nodiscard]] constexpr T lerp(T a, T b, S t) noexcept {
    return a + (b - a) * static_cast<T>(t);
}

/// Плавная интерполяция (smoothstep) по формуле 3t² - 2t³.
/// Даёт плавное начало и конец, без рывков.
template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr T smoothstep(T t) noexcept {
    t = std::clamp(t, T(0), T(1));
    return t * t * (T(3) - T(2) * t);
}

/// Ещё более плавная интерполяция (smootherstep) по формуле 6t⁵ - 15t⁴ + 10t³.
/// Первая и вторая производные равны нулю на концах.
template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr T smootherstep(T t) noexcept {
    t = std::clamp(t, T(0), T(1));
    return t * t * t * (t * (t * T(6) - T(15)) + T(10));
}

// ═══════════════════════════════════════════════════════════════
//  Зажим и сравнение
// ═══════════════════════════════════════════════════════════════

/// Зажимает значение между минимумом и максимумом.
/// (Обёртка над std::clamp для удобства).
template <typename T>
[[nodiscard]] constexpr T clamp(T value, T minVal, T maxVal) noexcept {
    return std::clamp(value, minVal, maxVal);
}

/// Проверяет, равны ли два числа с заданным абсолютным допуском.
template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr bool approxEqual(T a, T b, T tolerance = T(1e-10)) noexcept {
    return std::abs(a - b) <= tolerance;
}

/// Проверяет, близко ли число к нулю.
template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr bool isNearZero(T value, T tolerance = T(1e-10)) noexcept {
    return std::abs(value) <= tolerance;
}

/// Знак числа: +1, -1 или 0.
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T sign(T value) noexcept {
    return (value > T(0)) ? T(1) : ((value < T(0)) ? T(-1) : T(0));
}

// ═══════════════════════════════════════════════════════════════
//  Преобразования
// ═══════════════════════════════════════════════════════════════

/// Возводит число в квадрат (быстрее, чем std::pow(x, 2)).
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T sqr(T value) noexcept {
    return value * value;
}

/// Возводит число в куб.
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T cube(T value) noexcept {
    return value * value * value;
}

/// Линейное отображение: переводит value из диапазона [fromMin, fromMax]
/// в диапазон [toMin, toMax].
template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr T mapRange(T value, T fromMin, T fromMax, T toMin, T toMax) noexcept {
    T t = (value - fromMin) / (fromMax - fromMin);
    return lerp(toMin, toMax, t);
}

// ═══════════════════════════════════════════════════════════════
//  Тригонометрия с градусами (удобные обёртки)
// ═══════════════════════════════════════════════════════════════

/// Синус угла, заданного в градусах.
[[nodiscard]] inline double sinDegrees(double degrees) noexcept {
    return std::sin(degreesToRadians(degrees));
}

/// Косинус угла, заданного в градусах.
[[nodiscard]] inline double cosDegrees(double degrees) noexcept {
    return std::cos(degreesToRadians(degrees));
}

/// Тангенс угла, заданного в градусах.
[[nodiscard]] inline double tanDegrees(double degrees) noexcept {
    return std::tan(degreesToRadians(degrees));
}

} // namespace mir::math