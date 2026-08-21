
#pragma once

#include "../Core/Types/Scalar.hpp"
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace mir {

class Color {
public:

    Scalar r = 0.0;
    Scalar g = 0.0;
    Scalar b = 0.0;
    Scalar a = 1.0;

    constexpr Color() noexcept = default;

    constexpr Color(Scalar r, Scalar g, Scalar b, Scalar a = 1.0) noexcept
        : r(std::clamp(r, Scalar(0), Scalar(1)))
        , g(std::clamp(g, Scalar(0), Scalar(1)))
        , b(std::clamp(b, Scalar(0), Scalar(1)))
        , a(std::clamp(a, Scalar(0), Scalar(1)))
    {}

    [[nodiscard]] static constexpr Color rgb(Scalar r, Scalar g, Scalar b) noexcept {
        return Color{r, g, b, 1.0};
    }

    [[nodiscard]] static constexpr Color rgba(Scalar r, Scalar g, Scalar b, Scalar a) noexcept {
        return Color{r, g, b, a};
    }

    [[nodiscard]] static constexpr Color transparent() noexcept { return {0, 0, 0, 0}; }
    [[nodiscard]] static constexpr Color white()   noexcept { return {1, 1, 1, 1}; }
    [[nodiscard]] static constexpr Color black()   noexcept { return {0, 0, 0, 1}; }
    [[nodiscard]] static constexpr Color red()     noexcept { return {1, 0, 0, 1}; }
    [[nodiscard]] static constexpr Color green()   noexcept { return {0, 1, 0, 1}; }
    [[nodiscard]] static constexpr Color blue()    noexcept { return {0, 0, 1, 1}; }
    [[nodiscard]] static constexpr Color yellow()  noexcept { return {1, 1, 0, 1}; }
    [[nodiscard]] static constexpr Color cyan()    noexcept { return {0, 1, 1, 1}; }
    [[nodiscard]] static constexpr Color magenta() noexcept { return {1, 0, 1, 1}; }
    [[nodiscard]] static constexpr Color gray()    noexcept { return {0.5, 0.5, 0.5, 1}; }

    [[nodiscard]] std::string toHex() const {
        std::ostringstream oss;
        oss << "#"
            << std::hex << std::uppercase << std::setfill('0')
            << std::setw(2) << static_cast<int>(std::round(r * 255))
            << std::setw(2) << static_cast<int>(std::round(g * 255))
            << std::setw(2) << static_cast<int>(std::round(b * 255))
            << std::setw(2) << static_cast<int>(std::round(a * 255));
        return oss.str();
    }

    [[nodiscard]] static Color fromHex(const std::string& hex) {
        if (hex.empty() || hex[0] != '#') {
            throw std::invalid_argument("Color::fromHex: строка должна начинаться с '#'");
        }

        std::string digits = hex.substr(1);

        digits.erase(std::remove_if(digits.begin(), digits.end(), ::isspace), digits.end());

        Scalar r, g, b, a = 1.0;
        if (digits.size() == 3) {

            r = std::stoi(digits.substr(0,1), nullptr, 16) / 15.0;
            g = std::stoi(digits.substr(1,1), nullptr, 16) / 15.0;
            b = std::stoi(digits.substr(2,1), nullptr, 16) / 15.0;
        } else if (digits.size() == 4) {

            r = std::stoi(digits.substr(0,1), nullptr, 16) / 15.0;
            g = std::stoi(digits.substr(1,1), nullptr, 16) / 15.0;
            b = std::stoi(digits.substr(2,1), nullptr, 16) / 15.0;
            a = std::stoi(digits.substr(3,1), nullptr, 16) / 15.0;
        } else if (digits.size() == 6) {

            r = std::stoi(digits.substr(0,2), nullptr, 16) / 255.0;
            g = std::stoi(digits.substr(2,2), nullptr, 16) / 255.0;
            b = std::stoi(digits.substr(4,2), nullptr, 16) / 255.0;
        } else if (digits.size() == 8) {

            r = std::stoi(digits.substr(0,2), nullptr, 16) / 255.0;
            g = std::stoi(digits.substr(2,2), nullptr, 16) / 255.0;
            b = std::stoi(digits.substr(4,2), nullptr, 16) / 255.0;
            a = std::stoi(digits.substr(6,2), nullptr, 16) / 255.0;
        } else {
            throw std::invalid_argument("Color::fromHex: неверная длина HEX-строки (ожидается 3, 4, 6 или 8 цифр после #)");
        }
        return Color{r, g, b, a};
    }

    [[nodiscard]] constexpr Color withAlpha(Scalar newAlpha) const noexcept {
        return Color{r, g, b, newAlpha};
    }

    [[nodiscard]] static Color lerp(const Color& c1, const Color& c2, Scalar t) noexcept {
        t = std::clamp(t, Scalar(0), Scalar(1));
        return Color{
            c1.r + (c2.r - c1.r) * t,
            c1.g + (c2.g - c1.g) * t,
            c1.b + (c2.b - c1.b) * t,
            c1.a + (c2.a - c1.a) * t
        };
    }

    [[nodiscard]] Color lighter(Scalar factor = 0.3) const noexcept {
        return lerp(*this, white(), factor);
    }

    [[nodiscard]] Color darker(Scalar factor = 0.3) const noexcept {
        return lerp(*this, black(), factor);
    }

    [[nodiscard]] constexpr bool isTransparent() const noexcept {
        return a == 0.0;
    }

    [[nodiscard]] constexpr bool isOpaque() const noexcept {
        return a == 1.0;
    }

    friend constexpr bool operator==(const Color& a, const Color& b) noexcept {
        return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
    }
    friend constexpr bool operator!=(const Color& a, const Color& b) noexcept {
        return !(a == b);
    }
};

}