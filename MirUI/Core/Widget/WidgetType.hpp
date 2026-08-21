
#pragma once

#include <cstdint>

namespace MirUI {

enum class WidgetType : uint32_t {

    Window,
    Panel,
    DockPanel,
    Toolbar,
    Ribbon,
    Container,

    Button,
    Label,
    TextField,
    CheckBox,
    ComboBox,
    Slider,

    Tree,
    PropertyGrid,
    Viewport,
    Timeline,

    Unknown
};

}