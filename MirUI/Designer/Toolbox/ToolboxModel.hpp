
#pragma once

#include "ToolboxItem.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <optional>

namespace MirUI {

class ToolboxModel {
public:

    void addItem(const ToolboxItem& item) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const ToolboxItem& existing) { return existing.id == item.id; });
        if (it != m_items.end()) {
            *it = item;
        } else {
            m_items.push_back(item);
        }
    }

    bool removeItem(const std::string& id) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const ToolboxItem& item) { return item.id == id; });
        if (it != m_items.end()) {
            m_items.erase(it);
            return true;
        }
        return false;
    }

    [[nodiscard]] const ToolboxItem* findById(const std::string& id) const {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const ToolboxItem& item) { return item.id == id; });
        return (it != m_items.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const ToolboxItem* findByType(WidgetType type) const {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [type](const ToolboxItem& item) { return item.widgetType == type; });
        return (it != m_items.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const std::vector<ToolboxItem>& items() const {
        return m_items;
    }

    void clear() {
        m_items.clear();
    }

private:
    std::vector<ToolboxItem> m_items;
};

}