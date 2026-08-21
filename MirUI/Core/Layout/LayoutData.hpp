
#pragma once

#include "Size.hpp"
#include <string>

namespace MirUI {

enum class Unit {
    Pixel,
    Percent,
    Auto
};

enum class SizePolicy {
    Fixed,
    Fill,
    Fit,
    Stretch
};

struct LayoutData {

    double widthValue = 0.0;
    Unit   widthUnit  = Unit::Auto;

    double heightValue = 0.0;
    Unit   heightUnit  = Unit::Auto;

    Size minimumSize = { 0.0, 0.0 };
    Size maximumSize = { 1e9, 1e9 };

    SizePolicy horizontalPolicy = SizePolicy::Fixed;
    SizePolicy verticalPolicy   = SizePolicy::Fixed;

    static LayoutData fixed(double w, double h) {
        LayoutData ld;
        ld.widthValue = w;
        ld.widthUnit  = Unit::Pixel;
        ld.heightValue = h;
        ld.heightUnit  = Unit::Pixel;
        ld.horizontalPolicy = SizePolicy::Fixed;
        ld.verticalPolicy   = SizePolicy::Fixed;
        ld.minimumSize = {w, h};
        ld.maximumSize = {w, h};
        return ld;
    }

    static LayoutData fill() {
        LayoutData ld;
        ld.horizontalPolicy = SizePolicy::Fill;
        ld.verticalPolicy   = SizePolicy::Fill;
        return ld;
    }

    static LayoutData fit() {
        LayoutData ld;
        ld.widthUnit  = Unit::Auto;
        ld.heightUnit = Unit::Auto;
        ld.horizontalPolicy = SizePolicy::Fit;
        ld.verticalPolicy   = SizePolicy::Fit;
        return ld;
    }
};

}