// MirUI/Foundation/Metrics/MetricToken.hpp
// 📏 Семантические токены метрик — строковые ключи для всех размеров и отступов в теме.
//
// Вместо того чтобы писать в коде конкретные числа (например, 8 пикселей),
// мы используем смысловое имя — токен. Например, "spacingM" или "toolbarHeight".
// Тема MirUI хранит значения именно по таким ключам. Это позволяет легко
// настроить все размеры интерфейса, просто изменив значения в теме.
//
// Токены разделены на группы:
//   • Spacing   — отступы (XS, S, M, L, XL).
//   • Radii     — радиусы скруглений (S, M, L).
//   • Controls  — высоты элементов управления и панелей.
//   • Icons     — размеры иконок.
//   • Layout    — ширины панелей, отступы контейнеров.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string_view>

namespace MirUI::MetricToken {

// ── Отступы (Spacing) ────────────────────────────────────────
inline constexpr std::string_view SpacingXS  = "spacing.xs";   // 4 пикселя
inline constexpr std::string_view SpacingS   = "spacing.s";    // 8 пикселей
inline constexpr std::string_view SpacingM   = "spacing.m";    // 12 пикселей
inline constexpr std::string_view SpacingL   = "spacing.l";    // 16 пикселей
inline constexpr std::string_view SpacingXL  = "spacing.xl";   // 24 пикселя

// ── Радиусы скруглений ──────────────────────────────────────
inline constexpr std::string_view RadiusS    = "radius.s";     // 4 пикселя
inline constexpr std::string_view RadiusM    = "radius.m";     // 8 пикселей
inline constexpr std::string_view RadiusL    = "radius.l";     // 12 пикселей

// ── Элементы управления ─────────────────────────────────────
inline constexpr std::string_view ControlHeight  = "control.height";   // 28 пикселей
inline constexpr std::string_view ToolbarHeight  = "toolbar.height";   // 44 пикселя
inline constexpr std::string_view RibbonHeight   = "ribbon.height";    // 120 пикселей

// ── Иконки ──────────────────────────────────────────────────
inline constexpr std::string_view IconSmall   = "icon.small";    // 16 пикселей
inline constexpr std::string_view IconMedium  = "icon.medium";   // 24 пикселя
inline constexpr std::string_view IconLarge   = "icon.large";    // 32 пикселя

// ── Ширины панелей ──────────────────────────────────────────
inline constexpr std::string_view SidebarWidth    = "sidebar.width";     // 260 пикселей
inline constexpr std::string_view InspectorWidth  = "inspector.width";   // 280 пикселей

// ── Границы ─────────────────────────────────────────────────
inline constexpr std::string_view BorderWidth  = "border.width";    // 1 пиксель

// ── Контейнеры ──────────────────────────────────────────────
inline constexpr std::string_view PanelPadding  = "panel.padding";   // отступ внутри панелей
inline constexpr std::string_view PanelSpacing  = "panel.spacing";   // расстояние между детьми панели

} // namespace MirUI::MetricToken