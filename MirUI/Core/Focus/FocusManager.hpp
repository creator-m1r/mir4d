// MirUI/Core/Focus/FocusManager.hpp
// Manages keyboard focus among widgets.
// Pure C++23, no platform dependencies.

#pragma once

#include "../Widget/WidgetID.hpp"

namespace MirUI {

class FocusManager {
public:
    // Set focus to a specific widget.
    void setFocus(WidgetID id) {
        m_focusedWidget = id;
    }

    // Remove focus from any widget.
    void clearFocus() {
        m_focusedWidget = WidgetID{}; // becomes invalid (0)
    }

    // Returns the currently focused widget ID.
    // If no widget has focus, returns an invalid ID (value 0).
    [[nodiscard]] WidgetID focusedWidget() const {
        return m_focusedWidget;
    }

    // Move focus to the next widget in tab order (placeholder).
    void moveFocusNext() {
        // Will be implemented when focus chain is available.
    }

    // Move focus to the previous widget in tab order (placeholder).
    void moveFocusPrevious() {
        // Will be implemented when focus chain is available.
    }

private:
    WidgetID m_focusedWidget; // 0 means no focus
};

} // namespace MirUI