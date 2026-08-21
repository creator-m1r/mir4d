// MirUI/Workspace/PanelManager/PanelManager.hpp
// Менеджер панелей рабочего пространства.
// Хранит описание всех панелей (PanelDescriptor) и позволяет управлять их видимостью.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "PanelDescriptor.hpp"
#include <vector>
#include <string>
#include <optional>
#include <algorithm>

namespace MirUI {

class PanelManager {
public:
    // ── Регистрация и удаление панелей ───────────────────────

    // Добавить новую панель в менеджер.
    // Если панель с таким id уже существует, она будет заменена.
    void registerPanel(const PanelDescriptor& panel) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const PanelDescriptor& p) { return p.id == panel.id; });
        if (it != m_panels.end()) {
            *it = panel; // обновить существующую
        } else {
            m_panels.push_back(panel);
        }
    }

    // Удалить панель по её идентификатору.
    // Возвращает true, если панель была найдена и удалена.
    bool removePanel(const std::string& id) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const PanelDescriptor& p) { return p.id == id; });
        if (it != m_panels.end()) {
            m_panels.erase(it);
            return true;
        }
        return false;
    }

    // ── Управление видимостью ────────────────────────────────

    // Показать панель (установить visible = true).
    // Ничего не делает, если панель не найдена.
    void show(const std::string& id) {
        auto it = find(id);
        if (it) it->visible = true;
    }

    // Скрыть панель (установить visible = false).
    void hide(const std::string& id) {
        auto it = find(id);
        if (it) it->visible = false;
    }

    // Переключить видимость панели: если была видна — скрыть, и наоборот.
    void toggle(const std::string& id) {
        auto it = find(id);
        if (it) it->visible = !it->visible;
    }

    // Узнать, видна ли панель. Если панель не найдена, возвращает false.
    [[nodiscard]] bool isVisible(const std::string& id) const {
        auto it = findConst(id);
        return it ? it->visible : false;
    }

    // ── Поиск панели ─────────────────────────────────────────

    // Найти панель по id и вернуть указатель на неё.
    // Если панель не найдена, возвращает nullptr.
    [[nodiscard]] PanelDescriptor* find(const std::string& id) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const PanelDescriptor& p) { return p.id == id; });
        return (it != m_panels.end()) ? &(*it) : nullptr;
    }

    // Константная версия поиска.
    [[nodiscard]] const PanelDescriptor* find(const std::string& id) const {
        return findConst(id);
    }

    // ── Доступ ко всем панелям ───────────────────────────────

    // Получить ссылку на вектор всех зарегистрированных панелей.
    [[nodiscard]] const std::vector<PanelDescriptor>& panels() const {
        return m_panels;
    }

private:
    std::vector<PanelDescriptor> m_panels;

    // Вспомогательный метод для константного поиска (чтобы не дублировать код).
    [[nodiscard]] const PanelDescriptor* findConst(const std::string& id) const {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const PanelDescriptor& p) { return p.id == id; });
        return (it != m_panels.end()) ? &(*it) : nullptr;
    }
};

} // namespace MirUI