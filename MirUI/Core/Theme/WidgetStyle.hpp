
#pragma once

#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Typography/Font.hpp"
#include "WidgetStateStyle.hpp"
#include <string>

namespace MirUI {

enum class WidgetState {
    Normal,
    Hover,
    Pressed,
    Disabled,
    Focused,
    Selected
};

struct WidgetStyle {

    Color  background   = Color::transparent();
    Color  foreground   = Color::black();
    Color  border       = Color::transparent();
    Font   font         = Font("System", 14.0);
    double opacity      = 1.0;
    double cornerRadius = 0.0;
    ShadowData shadow;
    bool   visible      = true;

    WidgetStateStyle normal;
    WidgetStateStyle hover;
    WidgetStateStyle pressed;
    WidgetStateStyle disabled;
    WidgetStateStyle focused;
    WidgetStateStyle selected;

    bool operator==(const WidgetStyle& other) const {
        return background == other.background &&
               foreground == other.foreground &&
               border == other.border &&
               font == other.font &&
               opacity == other.opacity &&
               cornerRadius == other.cornerRadius &&
               shadow == other.shadow &&
               visible == other.visible &&
               normal == other.normal &&
               hover == other.hover &&
               pressed == other.pressed &&
               disabled == other.disabled &&
               focused == other.focused &&
               selected == other.selected;
    }

    bool operator!=(const WidgetStyle& other) const {
        return !(*this == other);
    }

    static WidgetStyle defaultButton() {
        WidgetStyle style;
        style.background   = Color::rgb(0.0f, 0.48f, 1.0f);
        style.foreground   = Color::white();
        style.cornerRadius = 8.0;
        style.font         = Font("System", 14.0, FontWeight::Medium);
        style.shadow       = ShadowData{Color::rgba(0,0,0,0.15f), 0, 2, 4};

        style.hover.background = Color::rgb(0.0f, 0.40f, 0.85f);

        style.pressed.background = Color::rgb(0.0f, 0.32f, 0.70f);

        style.disabled.background = Color::rgb(0.8f, 0.8f, 0.8f);
        style.disabled.foreground = Color::rgb(0.5f, 0.5f, 0.5f);
        style.disabled.opacity = 0.6f;

        return style;
    }
};

}