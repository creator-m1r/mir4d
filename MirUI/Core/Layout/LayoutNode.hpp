
#pragma once

#include "../Widget/WidgetID.hpp"
#include "Rect.hpp"
#include "Insets.hpp"

namespace MirUI {

enum class LayoutDirection {
    Horizontal,
    Vertical
};

struct LayoutNode {
    WidgetID widget;

    Rect rect;

    Insets margin;
    Insets padding;

    LayoutDirection direction = LayoutDirection::Vertical;
    double spacing = 0.0;
};

}