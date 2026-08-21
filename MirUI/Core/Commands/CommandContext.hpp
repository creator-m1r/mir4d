// MirUI/Core/Commands/CommandContext.hpp
// Execution context passed to command handlers.
// Pure C++23, no platform dependencies.

#pragma once

#include "../Widget/WidgetID.hpp"

namespace MirUI {

struct CommandContext {
    WidgetID sourceWidget;   // Widget that initiated the command (e.g., button)
    WidgetID focusedWidget;  // Currently focused widget
    WidgetID selectedWidget; // Currently selected widget (if applicable)

    // Future extensions: Project, Scene, Selection, Workspace, etc.
};

} // namespace MirUI