// MirUI/Foundation/Color/Color.hpp
// RGBA color structure with utility functions.
// Pure C++23, no platform dependencies.

#pragma once

#include <algorithm>

namespace MirUI {

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    constexpr Color() noexcept = default;
    constexpr Color(float r, float g, float b, float a = 1.0f) noexcept
        : r(r), g(g), b(b), a(a) {}

    // Named constructors
    static constexpr Color rgb(float r, float g, float b) noexcept {
        return { r, g, b, 1.0f };
    }
    static constexpr Color rgba(float r, float g, float b, float a) noexcept {
        return { r, g, b, a };
    }
    static constexpr Color transparent() noexcept { return { 0,0,0,0 }; }
    static constexpr Color white()   noexcept { return { 1,1,1,1 }; }
    static constexpr Color black()   noexcept { return { 0,0,0,1 }; }

    // Returns a copy with modified alpha.
    [[nodiscard]] constexpr Color withAlpha(float newAlpha) const noexcept {
        return { r, g, b, newAlpha };
    }

    // Linear interpolation between two colors.
    [[nodiscard]] static Color lerp(const Color& a, const Color& b, float t) noexcept {
        t = std::clamp(t, 0.0f, 1.0f);
        return {
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        };
    }

    bool operator==(const Color& other) const = default;
    bool operator!=(const Color& other) const = default;
};

} // namespace MirUI