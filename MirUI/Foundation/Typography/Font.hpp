
#pragma once

#include <string>

namespace MirUI {

enum class FontWeight {
    Thin = 100,
    ExtraLight = 200,
    Light = 300,
    Regular = 400,
    Medium = 500,
    SemiBold = 600,
    Bold = 700,
    ExtraBold = 800,
    Black = 900
};

enum class FontStyle {
    Normal,
    Italic,
    Oblique
};

struct Font {
    std::string family = "System";
    double size = 14.0;
    FontWeight weight = FontWeight::Regular;
    FontStyle style = FontStyle::Normal;

    constexpr Font() noexcept = default;

    Font(std::string family, double size,
         FontWeight weight = FontWeight::Regular,
         FontStyle style = FontStyle::Normal)
        : family(std::move(family)), size(size), weight(weight), style(style) {}

    bool operator==(const Font& other) const = default;
    bool operator!=(const Font& other) const = default;
};

}