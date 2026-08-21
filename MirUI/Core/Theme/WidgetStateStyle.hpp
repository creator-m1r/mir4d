
#pragma once

#include "../../Foundation/Color/Color.hpp"

namespace MirUI {

struct ShadowData {
    Color  color      = Color::transparent();
    double offsetX    = 0.0;
    double offsetY    = 2.0;
    double blurRadius = 4.0;

    bool operator==(const ShadowData& other) const {
        return color == other.color &&
               offsetX == other.offsetX &&
               offsetY == other.offsetY &&
               blurRadius == other.blurRadius;
    }
    bool operator!=(const ShadowData& other) const {
        return !(*this == other);
    }
};

struct WidgetStateStyle {
    Color background = Color::transparent();
    Color foreground = Color::black();
    Color border     = Color::transparent();

    double opacity = 1.0;

    ShadowData shadow;

    bool visible = true;

    bool operator==(const WidgetStateStyle& other) const {
        return background == other.background &&
               foreground == other.foreground &&
               border == other.border &&
               opacity == other.opacity &&
               shadow == other.shadow &&
               visible == other.visible;
    }
    bool operator!=(const WidgetStateStyle& other) const {
        return !(*this == other);
    }

    static WidgetStateStyle transparent() {
        return WidgetStateStyle{};
    }

    static WidgetStateStyle filled(const Color& bg) {
        WidgetStateStyle s;
        s.background = bg;
        s.foreground = Color::black();
        return s;
    }
};

}