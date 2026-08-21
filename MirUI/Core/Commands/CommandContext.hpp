
#pragma once

#include "../Widget/WidgetID.hpp"

namespace MirUI {

struct CommandContext {
    WidgetID sourceWidget;
    WidgetID focusedWidget;
    WidgetID selectedWidget;

};

}