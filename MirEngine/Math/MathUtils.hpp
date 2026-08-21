
#pragma once

#include <cmath>
#include <algorithm>
#include <numbers>
#include <type_traits>

namespace mir::math {

inline constexpr double PI = 3.14159265358979323846;

inline constexpr double TWO_PI = 2.0 * PI;

inline constexpr double HALF_PI = PI / 2.0;

inline constexpr double E = 2.71828182845904523536;

inline constexpr double GOLDEN_RATIO = 1.61803398874989484820;

inline constexpr double DEG_TO_RAD = PI / 180.0;

inline constexpr double RAD_TO_DEG = 180.0 / PI;

template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T degreesToRadians(T degrees) noexcept {
    return degrees * static_cast<T>(DEG_TO_RAD);
}

template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T radiansToDegrees(T radians) noexcept {
    return radians * static_cast<T>(RAD_TO_DEG);
}

template <typename T, typename S>
    requires std::is_arithmetic_v<T> && std::is_arithmetic_v<S>
[[nodiscard]] constexpr T lerp(T a, T b, S t) noexcept {
    return a + (b - a) * static_cast<T>(t);
}

template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr T smoothstep(T t) noexcept {
    t = std::clamp(t, T(0), T(1));
    return t * t * (T(3) - T(2) * t);
}

template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr T smootherstep(T t) noexcept {
    t = std::clamp(t, T(0), T(1));
    return t * t * t * (t * (t * T(6) - T(15)) + T(10));
}

template <typename T>
[[nodiscard]] constexpr T clamp(T value, T minVal, T maxVal) noexcept {
    return std::clamp(value, minVal, maxVal);
}

template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr bool approxEqual(T a, T b, T tolerance = T(1e-10)) noexcept {
    return std::abs(a - b) <= tolerance;
}

template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr bool isNearZero(T value, T tolerance = T(1e-10)) noexcept {
    return std::abs(value) <= tolerance;
}

template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T sign(T value) noexcept {
    return (value > T(0)) ? T(1) : ((value < T(0)) ? T(-1) : T(0));
}

template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T sqr(T value) noexcept {
    return value * value;
}

template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr T cube(T value) noexcept {
    return value * value * value;
}

template <typename T>
    requires std::is_floating_point_v<T>
[[nodiscard]] constexpr T mapRange(T value, T fromMin, T fromMax, T toMin, T toMax) noexcept {
    T t = (value - fromMin) / (fromMax - fromMin);
    return lerp(toMin, toMax, t);
}

[[nodiscard]] inline double sinDegrees(double degrees) noexcept {
    return std::sin(degreesToRadians(degrees));
}

[[nodiscard]] inline double cosDegrees(double degrees) noexcept {
    return std::cos(degreesToRadians(degrees));
}

[[nodiscard]] inline double tanDegrees(double degrees) noexcept {
    return std::tan(degreesToRadians(degrees));
}

}