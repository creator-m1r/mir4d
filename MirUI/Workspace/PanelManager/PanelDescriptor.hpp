
#pragma once

#include <string>
#include "../../Core/Layout/Size.hpp"
#include "../../Foundation/Icons/IconID.hpp"

namespace MirUI {

struct PanelDescriptor {

    std::string id;

    std::string title;

    IconID icon;

    bool visible = true;

    bool closable = true;

    bool floatable = true;

    Size minimumSize = { 200.0, 150.0 };

    Size preferredSize = { 300.0, 400.0 };
};

}