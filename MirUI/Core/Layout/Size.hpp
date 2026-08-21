
#pragma once

namespace MirUI {

struct Size {
    double width = 0.0;
    double height = 0.0;

    constexpr Size() noexcept = default;
    constexpr Size(double w, double h) noexcept : width(w), height(h) {}

    static constexpr Size zero() noexcept { return {}; }

    friend constexpr bool operator==(const Size& a, const Size& b) noexcept {
        return a.width == b.width && a.height == b.height;
    }
    friend constexpr bool operator!=(const Size& a, const Size& b) noexcept {
        return !(a == b);
    }
};

}