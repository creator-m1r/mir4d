
#pragma once

#include <string>
#include "../../Core/Widget/WidgetType.hpp"
#include "../../Foundation/Icons/IconID.hpp"

namespace MirUI {

struct ToolboxItem {
    std::string id;
    std::string name;
    WidgetType  widgetType;
    IconID      icon;
    std::string tooltip;
};

}