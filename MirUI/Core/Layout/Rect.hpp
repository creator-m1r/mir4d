
#pragma once

#include "Point.hpp"
#include "Size.hpp"
#include <algorithm>

namespace MirUI {

struct Rect {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;

    constexpr Rect() noexcept = default;
    constexpr Rect(double x, double y, double w, double h) noexcept
        : x(x), y(y), width(w), height(h) {}

    static constexpr Rect zero() noexcept { return {}; }

    [[nodiscard]] constexpr bool contains(const Point& p) const noexcept {
        return p.x >= x && p.x <= (x + width) &&
               p.y >= y && p.y <= (y + height);
    }

    [[nodiscard]] constexpr bool intersects(const Rect& other) const noexcept {
        return !(x + width < other.x || other.x + other.width < x ||
                 y + height < other.y || other.y + other.height < y);
    }

    [[nodiscard]] constexpr Point center() const noexcept {
        return { x + width * 0.5, y + height * 0.5 };
    }
    [[nodiscard]] constexpr Point topLeft() const noexcept {
        return { x, y };
    }
    [[nodiscard]] constexpr Point bottomRight() const noexcept {
        return { x + width, y + height };
    }

    [[nodiscard]] constexpr Rect unitedWith(const Rect& other) const noexcept {
        if (width <= 0 && height <= 0) return other;
        if (other.width <= 0 && other.height <= 0) return *this;

        double newX = std::min(x, other.x);
        double newY = std::min(y, other.y);
        double newRight = std::max(x + width, other.x + other.width);
        double newBottom = std::max(y + height, other.y + other.height);
        return Rect(newX, newY, newRight - newX, newBottom - newY);
    }

    friend constexpr bool operator==(const Rect& a, const Rect& b) noexcept {
        return a.x == b.x && a.y == b.y &&
               a.width == b.width && a.height == b.height;
    }
    friend constexpr bool operator!=(const Rect& a, const Rect& b) noexcept {
        return !(a == b);
    }
};

}