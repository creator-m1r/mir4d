// MirUI/Foundation/Color/Color.hpp
// RGBA color structure with utility functions.
// Pure C++23, no platform dependencies.

#pragma once

#include <algorithm>
#include <string>
#include <cstdio>

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

    // Parses "#RRGGBB" or "#RRGGBBAA" hex color.
    // Returns transparent black on malformed input.
    [[nodiscard]] static Color fromHex(const std::string& hex) noexcept {
        auto nibble = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        auto byteAt = [&](size_t index) -> float {
            int hi = nibble(hex[index]);
            int lo = nibble(hex[index + 1]);
            if (hi < 0 || lo < 0) return 0.0f;
            return static_cast<float>((hi << 4) | lo) / 255.0f;
        };

        if (hex.size() == 7 && hex[0] == '#') {
            return { byteAt(1), byteAt(3), byteAt(5), 1.0f };
        }
        if (hex.size() == 9 && hex[0] == '#') {
            return { byteAt(1), byteAt(3), byteAt(5), byteAt(7) };
        }
        return {};
    }

    // Serializes to "#RRGGBBAA".
    [[nodiscard]] std::string toHex() const noexcept {
        auto byte = [](float c) -> unsigned {
            return static_cast<unsigned>(std::clamp(c, 0.0f, 1.0f) * 255.0f + 0.5f);
        };
        char buf[10];
        std::snprintf(buf, sizeof buf, "#%02X%02X%02X%02X",
                      byte(r), byte(g), byte(b), byte(a));
        return std::string(buf);
    }

    bool operator==(const Color& other) const = default;
    bool operator!=(const Color& other) const = default;
};

} // namespace MirUI