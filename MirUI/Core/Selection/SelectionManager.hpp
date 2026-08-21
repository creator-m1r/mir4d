
#pragma once

#include "../Widget/WidgetID.hpp"
#include <vector>
#include <algorithm>

namespace MirUI {

class SelectionManager {
public:

    void select(WidgetID id) {

        if (m_selected.size() == 1 && m_selected[0] == id) {
            return;
        }

        m_selected.clear();
        m_selected.push_back(id);
    }

    void addToSelection(WidgetID id) {

        if (isSelected(id)) {
            return;
        }
        m_selected.push_back(id);
    }

    void deselect(WidgetID id) {

        auto it = std::remove(m_selected.begin(), m_selected.end(), id);
        m_selected.erase(it, m_selected.end());
    }

    void clear() {
        m_selected.clear();
    }

    [[nodiscard]] bool isSelected(WidgetID id) const {
        return std::find(m_selected.begin(), m_selected.end(), id) != m_selected.end();
    }

    [[nodiscard]] const std::vector<WidgetID>& selected() const {
        return m_selected;
    }

    [[nodiscard]] std::optional<WidgetID> singleSelected() const {
        if (m_selected.size() == 1) {
            return m_selected[0];
        }
        return std::nullopt;
    }

    [[nodiscard]] size_t count() const {
        return m_selected.size();
    }

    [[nodiscard]] bool empty() const {
        return m_selected.empty();
    }

private:
    std::vector<WidgetID> m_selected;
};

}