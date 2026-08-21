// MirUI/Core/Theme/WidgetStyle.hpp
// 🎨 Стиль виджета — полное описание внешнего вида для всех состояний.
//
// WidgetStyle содержит все визуальные параметры виджета: цвета, шрифты,
// отступы, тени, радиус скругления — и отдельные наборы этих параметров
// для каждого состояния (normal, hover, pressed, disabled, focused, selected).
//
// Благодаря этому одному C++ объекту, SwiftUI и WinUI могут нарисовать
// кнопку или панель одинаково, просто прочитав нужные значения для
// текущего состояния.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Typography/Font.hpp"
#include "WidgetStateStyle.hpp"
#include <string>

namespace MirUI {

// ── Состояния виджета ────────────────────────────────────────
enum class WidgetState {
    Normal,
    Hover,
    Pressed,
    Disabled,
    Focused,
    Selected
};

// ── Полный стиль виджета ────────────────────────────────────
struct WidgetStyle {
    // Базовые параметры (состояние Normal по умолчанию)
    Color  background   = Color::transparent();
    Color  foreground   = Color::black();
    Color  border       = Color::transparent();
    Font   font         = Font("System", 14.0);
    double opacity      = 1.0;
    double cornerRadius = 0.0;
    ShadowData shadow;
    bool   visible      = true;

    // Состояния — переопределяют базовые параметры
    WidgetStateStyle normal;
    WidgetStateStyle hover;
    WidgetStateStyle pressed;
    WidgetStateStyle disabled;
    WidgetStateStyle focused;
    WidgetStateStyle selected;

    // ── Операторы сравнения ──────────────────────────────────
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

    // ── Удобный конструктор для кнопки ──────────────────────
    static WidgetStyle defaultButton() {
        WidgetStyle style;
        style.background   = Color::rgb(0.0f, 0.48f, 1.0f);   // синий фон
        style.foreground   = Color::white();                   // белый текст
        style.cornerRadius = 8.0;
        style.font         = Font("System", 14.0, FontWeight::Medium);
        style.shadow       = ShadowData{Color::rgba(0,0,0,0.15f), 0, 2, 4};

        // Hover
        style.hover.background = Color::rgb(0.0f, 0.40f, 0.85f);

        // Pressed
        style.pressed.background = Color::rgb(0.0f, 0.32f, 0.70f);

        // Disabled
        style.disabled.background = Color::rgb(0.8f, 0.8f, 0.8f);
        style.disabled.foreground = Color::rgb(0.5f, 0.5f, 0.5f);
        style.disabled.opacity = 0.6f;

        return style;
    }
};

} // namespace MirUI