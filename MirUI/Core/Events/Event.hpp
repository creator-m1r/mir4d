// MirUI/Core/Events/Event.hpp
// Base event structure. Platform-independent.
// Pure C++23, no platform dependencies.

#pragma once

#include "EventType.hpp"
#include "../Widget/WidgetID.hpp"
#include <cstdint>

namespace MirUI {

struct Event {
    EventType type{};
    WidgetID target{};
    bool handled = false;

    // Generic payload slots for lightweight application events.
    // SelectionChanged uses data0 = selection kind, data1 = selection ID.
    std::uint64_t data0 = 0;
    std::uint64_t data1 = 0;
};

} // namespace MirUI
