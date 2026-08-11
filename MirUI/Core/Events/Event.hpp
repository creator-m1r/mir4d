// MirUI/Core/Events/Event.hpp
// Base event structure. Platform-independent.
// Pure C++23, no platform dependencies.

#pragma once

#include "EventType.hpp"
#include "../Widget/WidgetID.hpp"

namespace MirUI {

struct Event {
    EventType type;
    WidgetID target;   // Widget that should receive the event initially.
    bool handled = false; // Set to true to stop propagation.
};

} // namespace MirUI