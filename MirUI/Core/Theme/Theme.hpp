// MirUI/Core/Theme/Theme.hpp
// 🎨 Тема MirUI — чистое описание внешнего вида интерфейса (только данные).
//
// Theme — это "паспорт" визуального стиля всего приложения.
// Она НЕ содержит логики загрузки, сохранения или переключения —
// это делают ThemeManager и UIProjectSerializer.
//
// Тема объединяет в себе:
//   • id и name                — уникальный идентификатор и человекочитаемое имя.
//   • colors (ColorPalette)    — все цвета интерфейса и вьюпорта.
//   • metrics (Metrics)        — размеры, отступы, радиусы.
//   • typography (Typography)  — шрифты для разных элементов.
//   • animations (AnimationSettings) — настройки анимаций по умолчанию.
//
// Создавать тему можно либо через конструктор, либо через статические
// фабричные методы (createLight, createDark). В будущем темы будут
// загружаться из .mirtheme файлов через UIProjectSerializer.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "ThemeID.hpp"
#include "../../Foundation/Color/ColorPalette.hpp"
#include "../../Foundation/Metrics/Metrics.hpp"
#include "../../Foundation/Typography/Typography.hpp"
#include "../../Foundation/Animation/AnimationSpec.hpp"
#include <string>

namespace MirUI {

struct AnimationSettings {
    double defaultDuration = 0.25;
    bool   enableAnimations = true;

    bool operator==(const AnimationSettings& other) const {
        return defaultDuration == other.defaultDuration &&
               enableAnimations == other.enableAnimations;
    }
    bool operator!=(const AnimationSettings& other) const {
        return !(*this == other);
    }
};

struct Theme {
    ThemeID id;
    std::string name;

    ColorPalette colors;
    Metrics      metrics;
    Typography   typography;
    AnimationSettings animations;

    // Явное сравнение всех полей
    bool operator==(const Theme& other) const {
        return id == other.id &&
               name == other.name &&
               colors == other.colors &&
               metrics == other.metrics &&
               typography == other.typography &&
               animations == other.animations;
    }
    bool operator!=(const Theme& other) const {
        return !(*this == other);
    }

    static Theme createLight() {
        Theme theme;
        theme.id = ThemeID("mir.light");
        theme.name = "Светлая тема";
        theme.colors = ColorPalette::light();
        theme.metrics = Metrics::standard();
        theme.typography = Typography::standard();
        theme.animations = AnimationSettings{};
        return theme;
    }

    static Theme createDark() {
        Theme theme;
        theme.id = ThemeID("mir.dark");
        theme.name = "Тёмная тема";
        theme.colors = ColorPalette::dark();
        theme.metrics = Metrics::standard();
        theme.typography = Typography::standard();
        theme.animations = AnimationSettings{};
        return theme;
    }
};

} // namespace MirUI