
#pragma once

#include "../Widget/WidgetID.hpp"

namespace MirUI {

class FocusManager {
public:

    void setFocus(WidgetID id) {
        m_focusedWidget = id;
    }

    void clearFocus() {
        m_focusedWidget = WidgetID{};
    }

    [[nodiscard]] WidgetID focusedWidget() const {
        return m_focusedWidget;
    }

    void moveFocusNext() {

    }

    void moveFocusPrevious() {

    }

private:
    WidgetID m_focusedWidget;
};

}