// MirUI/Core/Commands/Command.hpp
// Descriptor of a command with its metadata.
// Pure C++23, no platform dependencies.

#pragma once

#include "CommandID.hpp"
#include <string>

namespace MirUI {

struct Command {
    CommandID id;

    std::string title;       // human-readable name (e.g. "Select")
    std::string description; // tooltip / description
    std::string icon;        // icon identifier (platform-agnostic string)

    bool enabled = true;
    bool checked = false;    // for toggle commands
};

} // namespace MirUI