// MirUI/Designer/Toolbox/ToolboxModel.hpp
// 📦 Модель данных для панели инструментов (Toolbox).
//
// Хранит список всех доступных элементов (ToolboxItem), которые пользователь
// может перетащить на холст, чтобы создать новый виджет. Модель не зависит
// от платформы и не содержит логики отображения — только данные.
//
// Основные возможности:
//   • Добавлять, удалять элементы.
//   • Искать элемент по идентификатору или типу виджета.
//   • Получать полный список элементов (для построения интерфейса тулбокса).
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "ToolboxItem.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <optional>

namespace MirUI {

class ToolboxModel {
public:
    // ── Добавление и удаление элементов ──────────────────────

    // Добавляет новый элемент в модель. Если элемент с таким id уже существует, заменяет его.
    void addItem(const ToolboxItem& item) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const ToolboxItem& existing) { return existing.id == item.id; });
        if (it != m_items.end()) {
            *it = item; // замена
        } else {
            m_items.push_back(item);
        }
    }

    // Удаляет элемент по идентификатору. Возвращает true, если элемент был найден и удалён.
    bool removeItem(const std::string& id) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const ToolboxItem& item) { return item.id == id; });
        if (it != m_items.end()) {
            m_items.erase(it);
            return true;
        }
        return false;
    }

    // ── Поиск ─────────────────────────────────────────────────

    // Ищет элемент по строковому идентификатору. Возвращает указатель или nullptr.
    [[nodiscard]] const ToolboxItem* findById(const std::string& id) const {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const ToolboxItem& item) { return item.id == id; });
        return (it != m_items.end()) ? &(*it) : nullptr;
    }

    // Ищет элемент по типу виджета (WidgetType). Возвращает указатель или nullptr.
    [[nodiscard]] const ToolboxItem* findByType(WidgetType type) const {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [type](const ToolboxItem& item) { return item.widgetType == type; });
        return (it != m_items.end()) ? &(*it) : nullptr;
    }

    // ── Доступ ко всем элементам ─────────────────────────────
    [[nodiscard]] const std::vector<ToolboxItem>& items() const {
        return m_items;
    }

    // ── Очистка модели ───────────────────────────────────────
    void clear() {
        m_items.clear();
    }

private:
    std::vector<ToolboxItem> m_items;
};

} // namespace MirUI