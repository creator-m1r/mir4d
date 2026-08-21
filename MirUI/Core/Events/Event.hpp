
#pragma once

#include "EventType.hpp"
#include "../Widget/WidgetID.hpp"
#include <cstdint>

namespace MirUI {

struct Event {
    EventType type{};
    WidgetID target{};
    bool handled = false;

    std::uint64_t data0 = 0;
    std::uint64_t data1 = 0;
};

}
