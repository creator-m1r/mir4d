
#pragma once

#include "WidgetType.hpp"
#include "../Layout/Rect.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace MirUI {

using WidgetID = std::string;

struct WidgetSnapshot {

    WidgetID   id;
    WidgetType type = WidgetType::Unknown;
    std::string name;

    bool visible = true;
    bool enabled = true;

    Rect bounds;

    std::unordered_map<std::string, std::string> properties;

    std::vector<WidgetSnapshot> children;

    bool operator==(const WidgetSnapshot& other) const {
        return id == other.id &&
               type == other.type &&
               name == other.name &&
               visible == other.visible &&
               enabled == other.enabled &&
               bounds == other.bounds &&
               properties == other.properties &&
               children == other.children;
    }
    bool operator!=(const WidgetSnapshot& other) const {
        return !(*this == other);
    }
};

}