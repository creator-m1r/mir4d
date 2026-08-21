
#pragma once

#include "WidgetSnapshot.hpp"
#include <cstddef>

namespace MirUI {

struct WidgetTreeSnapshot {
    WidgetSnapshot root;
    std::size_t     totalNodes = 0;

    bool operator==(const WidgetTreeSnapshot& other) const {
        return root == other.root && totalNodes == other.totalNodes;
    }

    bool operator!=(const WidgetTreeSnapshot& other) const {
        return !(*this == other);
    }
};

}