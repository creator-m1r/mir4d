
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

}