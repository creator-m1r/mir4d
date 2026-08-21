
#pragma once

#include "WindowDescriptor.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <optional>
#include <stdexcept>

namespace MirUI {

class WindowManager {
public:

    void createWindow(const WindowDescriptor& descriptor) {

        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == descriptor.id; });
        if (it != m_windows.end()) {
            *it = descriptor;
        } else {
            m_windows.push_back(descriptor);
        }

        if (m_windows.size() == 1) {
            m_activeWindow = descriptor.id;
        }
    }

    void closeWindow(const std::string& id) {
        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == id; });
        if (it == m_windows.end()) {
            return;
        }

        m_windows.erase(it);

        if (m_activeWindow == id) {
            if (!m_windows.empty()) {
                m_activeWindow = m_windows.front().id;
            } else {
                m_activeWindow.reset();
            }
        }
    }

    void activateWindow(const std::string& id) {
        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == id; });
        if (it == m_windows.end()) {
            throw std::runtime_error("WindowManager::activateWindow: окно с id '" + id + "' не найдено.");
        }
        m_activeWindow = id;
    }

    [[nodiscard]] std::optional<std::string> activeWindow() const {
        return m_activeWindow;
    }

    [[nodiscard]] WindowDescriptor* find(const std::string& id) {
        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == id; });
        return (it != m_windows.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const WindowDescriptor* find(const std::string& id) const {
        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == id; });
        return (it != m_windows.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const std::vector<WindowDescriptor>& windows() const {
        return m_windows;
    }

private:
    std::vector<WindowDescriptor> m_windows;
    std::optional<std::string> m_activeWindow;
};

}