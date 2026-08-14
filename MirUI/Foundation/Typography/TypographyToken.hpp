// MirUI/Foundation/Typography/TypographyToken.hpp
// 🔤 Семантические токены типографики — строковые ключи для всех текстовых стилей в теме.
//
// Вместо того чтобы писать в коде конкретный шрифт (например, "System 14 Bold"),
// мы используем смысловое имя — токен. Например, "body" или "dimension".
// Тема MirUI хранит шрифты именно по таким ключам. Это позволяет легко
// изменить шрифт всех заголовков или размеров во всём интерфейсе.
//
// Токены разделены на группы:
//   • Interface   — заголовки, тело, подписи, кнопки, код.
//   • Engineering — специфичные для САПР: размеры, аннотации, координаты.
//   • Status      — командная строка, статусная панель.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string_view>

namespace MirUI::TypographyToken {

// ── Интерфейс ────────────────────────────────────────────────
inline constexpr std::string_view Display      = "typography.display";      // самый крупный заголовок
inline constexpr std::string_view Title        = "typography.title";        // заголовок окна/панели
inline constexpr std::string_view Subtitle     = "typography.subtitle";     // подзаголовок
inline constexpr std::string_view Body         = "typography.body";         // основной текст
inline constexpr std::string_view BodyStrong   = "typography.bodyStrong";   // полужирный основной текст
inline constexpr std::string_view Caption      = "typography.caption";      // мелкий текст (подписи)
inline constexpr std::string_view Button       = "typography.button";       // текст на кнопках
inline constexpr std::string_view Toolbar      = "typography.toolbar";      // текст в панели инструментов
inline constexpr std::string_view PropertyName = "typography.propertyName"; // имена свойств в инспекторе
inline constexpr std::string_view PropertyValue= "typography.propertyValue";// значения свойств
inline constexpr std::string_view Code         = "typography.code";         // моноширинный шрифт для кода

// ── Инженерные (CAD) ────────────────────────────────────────
inline constexpr std::string_view Dimension    = "typography.dimension";    // размеры (dimension)
inline constexpr std::string_view Annotation   = "typography.annotation";   // аннотации
inline constexpr std::string_view Coordinate   = "typography.coordinate";   // координаты (X, Y, Z, t)
inline constexpr std::string_view Status       = "typography.status";       // статусная строка
inline constexpr std::string_view Command      = "typography.command";      // командная строка

} // namespace MirUI::TypographyToken