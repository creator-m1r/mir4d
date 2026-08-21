
#pragma once

#include "Theme.hpp"
#include "ThemeID.hpp"
#include <unordered_map>
#include <functional>
#include <string>
#include <optional>

namespace MirUI {

class ThemeManager {
public:

    ThemeManager() {

        Theme defaultLight = Theme::createLight();
        registerTheme(defaultLight);
        setTheme(defaultLight.id);
    }

    [[nodiscard]] Theme current() const {
        auto it = m_themes.find(m_currentId);
        if (it != m_themes.end()) {
            return it->second;
        }

        return Theme::createLight();
    }

    void setTheme(const Theme& theme) {

        m_themes[theme.id] = theme;

        m_currentId = theme.id;

        notifyListeners(theme.id);
    }

    void setTheme(const ThemeID& id) {
        auto it = m_themes.find(id);
        if (it != m_themes.end()) {
            m_currentId = id;
            notifyListeners(id);
        }
    }

    [[nodiscard]] ThemeID currentThemeID() const {
        return m_currentId;
    }

    void registerTheme(const Theme& theme) {
        m_themes[theme.id] = theme;
    }

    [[nodiscard]] bool hasTheme(const ThemeID& id) const {
        return m_themes.find(id) != m_themes.end();
    }

    void resetToDefault() {
        Theme defaultLight = Theme::createLight();
        registerTheme(defaultLight);
        setTheme(defaultLight.id);
    }

    using ThemeChangeCallback = std::function<void(const ThemeID& newThemeId)>;

    void onThemeChanged(ThemeChangeCallback callback) {
        m_listeners.push_back(std::move(callback));
    }

private:

    std::unordered_map<ThemeID, Theme, std::hash<ThemeID>> m_themes;

    ThemeID m_currentId;

    std::vector<ThemeChangeCallback> m_listeners;

    void notifyListeners(const ThemeID& newId) {
        for (auto& callback : m_listeners) {
            if (callback) {
                callback(newId);
            }
        }
    }
};

}