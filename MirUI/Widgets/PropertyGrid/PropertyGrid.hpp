// MirUI/Widgets/PropertyGrid/PropertyGrid.hpp
// Виджет «Панель свойств» (PropertyGrid) — показывает список редактируемых свойств,
// обычно в инспекторе объектов. Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Widget/Widget.hpp"
#include "Property.hpp"
#include <vector>
#include <optional>
#include <algorithm>
#include <string>

namespace MirUI {

class PropertyGrid : public Widget {
public:
    // Создаём виджет с типом PropertyGrid.
    PropertyGrid()
        : Widget(WidgetType::PropertyGrid)
    {}

    // ── Добавление и удаление свойств ────────────────────────

    // Добавить новое свойство в панель.
    // Если свойство с таким id уже существует, оно будет заменено.
    void addProperty(const Property& property) {
        // Ищем, нет ли уже свойства с таким же id.
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
            [&](const Property& p) { return p.id == property.id; });
        if (it != m_properties.end()) {
            // Заменяем существующее свойство новым значением.
            *it = property;
        } else {
            // Добавляем новое свойство.
            m_properties.push_back(property);
        }
    }

    // Удалить свойство по его идентификатору.
    // Возвращает true, если свойство было найдено и удалено.
    bool removeProperty(const std::string& id) {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
            [&](const Property& p) { return p.id == id; });
        if (it != m_properties.end()) {
            m_properties.erase(it);
            return true;
        }
        return false;
    }

    // ── Доступ к свойствам ───────────────────────────────────

    // Получить список всех свойств (только для чтения).
    [[nodiscard]] const std::vector<Property>& properties() const {
        return m_properties;
    }

    // ── Чтение и запись значений ─────────────────────────────

    // Установить новое значение свойства по его id.
    // Возвращает true, если свойство найдено и значение обновлено.
    bool setValue(const std::string& id, const StateValue& newValue) {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
            [&](const Property& p) { return p.id == id; });
        if (it != m_properties.end()) {
            // Если свойство помечено как readOnly, не даём его изменить.
            if (it->readOnly) {
                return false;
            }
            it->value = newValue;
            return true;
        }
        return false;
    }

    // Получить значение свойства по его id.
    // Возвращает std::nullopt, если свойство не найдено.
    [[nodiscard]] std::optional<StateValue> value(const std::string& id) const {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
            [&](const Property& p) { return p.id == id; });
        if (it != m_properties.end()) {
            return it->value;
        }
        return std::nullopt;
    }

private:
    std::vector<Property> m_properties; // Все свойства, отображаемые в панели.
};

} // namespace MirUI