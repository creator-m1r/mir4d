// MirUI/Core/Theme/WidgetStateStyle.hpp
// 🎭 Стиль для одного состояния виджета — описывает, как выглядит виджет,
//    когда он в состоянии покоя, наведения, нажатия, фокуса или отключён.
//
// Каждый WidgetStyle (полный стиль виджета) содержит в себе несколько
// WidgetStateStyle — по одному на каждое возможное состояние:
//   • normal   — обычное состояние (пользователь не взаимодействует).
//   • hover    — курсор мыши наведён на виджет.
//   • pressed  — кнопка мыши нажата на виджете.
//   • active   — виджет активен (например, открытое меню).
//   • disabled — виджет заблокирован и не реагирует на действия.
//   • focused  — виджет находится в фокусе клавиатуры.
//   • selected — виджет выделен (например, в списке).
//
// Каждое состояние может переопределять:
//   • background — цвет фона.
//   • foreground — цвет текста / содержимого.
//   • border     — цвет рамки.
//   • opacity    — прозрачность (0.0 = невидим, 1.0 = полностью видим).
//   • shadow     — тень.
//   • visible    — видимость (если false, виджет скрыт в этом состоянии).
//
// Благодаря этой структуре MirUI может описать ЛЮБОЙ виджет для ЛЮБОЙ платформы
// одним и тем же C++ объектом, а SwiftUI/WinUI просто прочитают значения
// и применят их к нативным элементам.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Foundation/Color/Color.hpp"

namespace MirUI {

// ── Упрощённая структура тени (будет расширена в Foundation/Shadow) ──
struct ShadowData {
    Color  color      = Color::transparent();
    double offsetX    = 0.0;
    double offsetY    = 2.0;
    double blurRadius = 4.0;

    // Операторы сравнения для ShadowData
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
    Color background = Color::transparent();  // цвет фона в этом состоянии
    Color foreground = Color::black();        // цвет текста / содержимого
    Color border     = Color::transparent();  // цвет рамки

    double opacity = 1.0;                     // общая прозрачность (0..1)

    ShadowData shadow;                        // тень (none, subtle, floating…)

    bool visible = true;                      // видимость в этом состоянии

    // ── Операторы сравнения (явная реализация) ──────────────
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

    // ── Статические методы для удобства ──────────────────────
    // Стиль, который ничего не меняет (прозрачный фон, нет тени, visible=true).
    static WidgetStateStyle transparent() {
        return WidgetStateStyle{};
    }

    // Стиль с заданным фоном и чёрным текстом.
    static WidgetStateStyle filled(const Color& bg) {
        WidgetStateStyle s;
        s.background = bg;
        s.foreground = Color::black();
        return s;
    }
};

} // namespace MirUI