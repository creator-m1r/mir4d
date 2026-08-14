// MirUI/Core/Layout/LayoutNode.hpp
// Describes the layout of a single widget: its geometry, margins, padding, direction, and spacing.
// Pure C++23, no platform dependencies.

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
    WidgetID widget;   // The widget this node lays out.

    Rect rect;         // Final computed rectangle for the widget.

    Insets margin;     // External spacing around the widget.
    Insets padding;    // Internal spacing inside the widget.

    LayoutDirection direction = LayoutDirection::Vertical; // Child layout direction.
    double spacing = 0.0; // Spacing between children.
};

} // namespace MirUI