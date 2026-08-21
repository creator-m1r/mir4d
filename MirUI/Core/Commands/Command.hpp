
#pragma once

#include "CommandID.hpp"
#include <string>

namespace MirUI {

struct Command {
    CommandID id;

    std::string title;
    std::string description;
    std::string icon;

    bool enabled = true;
    bool checked = false;
};

}