
#pragma once

#include "PanelDescriptor.hpp"
#include <vector>
#include <string>
#include <optional>
#include <algorithm>

namespace MirUI {

class PanelManager {
public:

    void registerPanel(const PanelDescriptor& panel) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const PanelDescriptor& p) { return p.id == panel.id; });
        if (it != m_panels.end()) {
            *it = panel;
        } else {
            m_panels.push_back(panel);
        }
    }

    bool removePanel(const std::string& id) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const PanelDescriptor& p) { return p.id == id; });
        if (it != m_panels.end()) {
            m_panels.erase(it);
            return true;
        }
        return false;
    }

    void show(const std::string& id) {
        auto it = find(id);
        if (it) it->visible = true;
    }

    void hide(const std::string& id) {
        auto it = find(id);
        if (it) it->visible = false;
    }

    void toggle(const std::string& id) {
        auto it = find(id);
        if (it) it->visible = !it->visible;
    }

    [[nodiscard]] bool isVisible(const std::string& id) const {
        auto it = findConst(id);
        return it ? it->visible : false;
    }

    [[nodiscard]] PanelDescriptor* find(const std::string& id) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const PanelDescriptor& p) { return p.id == id; });
        return (it != m_panels.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const PanelDescriptor* find(const std::string& id) const {
        return findConst(id);
    }

    [[nodiscard]] const std::vector<PanelDescriptor>& panels() const {
        return m_panels;
    }

private:
    std::vector<PanelDescriptor> m_panels;

    [[nodiscard]] const PanelDescriptor* findConst(const std::string& id) const {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const PanelDescriptor& p) { return p.id == id; });
        return (it != m_panels.end()) ? &(*it) : nullptr;
    }
};

}