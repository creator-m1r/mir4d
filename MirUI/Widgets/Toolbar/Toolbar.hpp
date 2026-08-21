
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <vector>
#include <algorithm>

namespace MirUI {

enum class ToolbarOrientation {
    Horizontal,
    Vertical
};

class Toolbar : public Widget {
public:
    explicit Toolbar()
        : Widget(WidgetType::Toolbar)
    {}

    void addItem(WidgetID id) {
        if (std::find(m_items.begin(), m_items.end(), id) == m_items.end()) {
            m_items.push_back(id);
        }
    }

    void removeItem(WidgetID id) {
        auto it = std::remove(m_items.begin(), m_items.end(), id);
        m_items.erase(it, m_items.end());
    }

    [[nodiscard]] const std::vector<WidgetID>& items() const { return m_items; }

    void setOrientation(ToolbarOrientation orientation) { m_orientation = orientation; }
    [[nodiscard]] ToolbarOrientation orientation() const { return m_orientation; }

private:
    std::vector<WidgetID> m_items;
    ToolbarOrientation m_orientation = ToolbarOrientation::Horizontal;
};

}