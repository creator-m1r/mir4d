// MirUI/Core/Layout/Point.hpp
// 2D point with arithmetic operations.
// Pure C++23, no platform dependencies.

#pragma once

namespace MirUI {

struct Point {
    double x = 0.0;
    double y = 0.0;

    constexpr Point() noexcept = default;
    constexpr Point(double x, double y) noexcept : x(x), y(y) {}

    static constexpr Point zero() noexcept { return {}; }

    // Arithmetic operators
    friend constexpr Point operator+(const Point& a, const Point& b) noexcept {
        return { a.x + b.x, a.y + b.y };
    }
    friend constexpr Point operator-(const Point& a, const Point& b) noexcept {
        return { a.x - b.x, a.y - b.y };
    }
    friend constexpr bool operator==(const Point& a, const Point& b) noexcept {
        return a.x == b.x && a.y == b.y;
    }
    friend constexpr bool operator!=(const Point& a, const Point& b) noexcept {
        return !(a == b);
    }
};

} // namespace MirUI