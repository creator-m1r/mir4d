
#pragma once

namespace MirUI {

struct Insets {
    double top = 0.0;
    double right = 0.0;
    double bottom = 0.0;
    double left = 0.0;

    constexpr Insets() noexcept = default;
    constexpr Insets(double all) noexcept
        : top(all), right(all), bottom(all), left(all) {}
    constexpr Insets(double top, double right, double bottom, double left) noexcept
        : top(top), right(right), bottom(bottom), left(left) {}

    static constexpr Insets zero() noexcept { return {}; }

    friend constexpr bool operator==(const Insets& a, const Insets& b) noexcept {
        return a.top == b.top && a.right == b.right &&
               a.bottom == b.bottom && a.left == b.left;
    }
    friend constexpr bool operator!=(const Insets& a, const Insets& b) noexcept {
        return !(a == b);
    }
};

}