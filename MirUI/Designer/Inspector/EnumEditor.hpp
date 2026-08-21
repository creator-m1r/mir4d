// MirUI/Designer/Inspector/EnumEditor.hpp
// 📋 Редактор перечислений для инспектора свойств (PropertyGrid).
//
// Многие свойства виджетов могут принимать только определённые значения
// из фиксированного списка — например, «выравнивание текста» может быть
// Left, Center, Right. EnumEditor управляет таким списком и позволяет
// выбрать одно значение через выпадающий список (combo box).
//
// При изменении выбранного значения редактор создаёт команду
// ChangePropertyCommand и выполняет её через историю документа,
// автоматически поддерживая Undo/Redo. Сам EnumEditor не рисует
// выпадающий список — только хранит логику и значения. Отображение
// выполняется рендерером (SwiftUI, WinUI, WebUI).
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/State/StateValue.hpp"
#include "../Document/UIDocument.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <optional>

namespace MirUI {

class EnumEditor {
public:
    // Конструктор:
    //   doc           — документ, содержащий виджет
    //   widgetId      — ID редактируемого виджета
    //   propertyName  — имя свойства (например, "alignment")
    //   possibleValues — список всех допустимых значений (строки)
    EnumEditor(UIDocument& doc,
               WidgetID widgetId,
               const std::string& propertyName,
               const std::vector<std::string>& possibleValues)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
        , m_possibleValues(possibleValues)
    {
        // Загружаем текущее значение из виджета.
        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            auto value = widget->getProperty(propertyName);
            if (value.has_value() && std::holds_alternative<std::string>(*value)) {
                m_currentValue = std::get<std::string>(*value);
            }
        }

        // Если загруженное значение отсутствует или не входит в список,
        // устанавливаем первый элемент списка как значение по умолчанию.
        if (!m_possibleValues.empty()) {
            if (!isValueValid(m_currentValue)) {
                m_currentValue = m_possibleValues.front();
                // Не выполняем команду при инициализации, просто запоминаем.
                // Чтобы значение сохранилось в виджете, нужно явно вызвать setValue.
            }
        }
    }

    // ── Текущее значение ─────────────────────────────────────
    [[nodiscard]] const std::string& currentValue() const {
        return m_currentValue;
    }

    // ── Список возможных значений ────────────────────────────
    [[nodiscard]] const std::vector<std::string>& possibleValues() const {
        return m_possibleValues;
    }

    // ── Установка нового значения ────────────────────────────
    // Вызывается, когда пользователь выбирает пункт из выпадающего списка.
    // Создаёт команду изменения свойства и выполняет её.
    void setValue(const std::string& newValue) {
        if (newValue == m_currentValue) return; // без изменений

        // Проверяем, что новое значение допустимо.
        if (!isValueValid(newValue)) {
            return; // значение не из списка — игнорируем
        }

        // Создаём команду для изменения свойства.
        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName,
            StateValue(newValue)
        );
        // Выполняем команду — она попадёт в историю Undo/Redo.
        m_doc.history().execute(std::move(cmd));

        // Обновляем локальное значение.
        m_currentValue = newValue;
    }

    // ── Индекс текущего значения в списке ────────────────────
    // Удобно для передачи в рендерер (selectedIndex).
    [[nodiscard]] int currentIndex() const {
        auto it = std::find(m_possibleValues.begin(), m_possibleValues.end(), m_currentValue);
        if (it != m_possibleValues.end()) {
            return static_cast<int>(std::distance(m_possibleValues.begin(), it));
        }
        return 0; // fallback
    }

    // ── Сброс к значению по умолчанию ────────────────────────
    void resetToDefault() {
        if (!m_possibleValues.empty()) {
            setValue(m_possibleValues.front());
        }
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    std::vector<std::string> m_possibleValues;
    std::string m_currentValue;

    // Проверяет, входит ли значение в список допустимых.
    [[nodiscard]] bool isValueValid(const std::string& value) const {
        return std::find(m_possibleValues.begin(), m_possibleValues.end(), value)
               != m_possibleValues.end();
    }
};

} // namespace MirUI