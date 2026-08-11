// MirUI/Designer/Inspector/StringEditor.hpp
// 📝 Редактор строкового свойства — текстовое поле в инспекторе.
//
// Самый часто используемый редактор: имя виджета, текст кнопки,
// идентификатор команды — всё это строки. StringEditor управляет
// одним строковым значением: хранит его и позволяет изменить.
//
// Как и другие редакторы, он не рисует поле ввода сам, а только
// предоставляет данные и метод setValue(). При изменении значения
// создаётся ChangePropertyCommand, которая выполняется через историю
// документа, поэтому любое изменение можно отменить через Ctrl+Z.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <string>

namespace MirUI {

class StringEditor {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Принимает документ, ID виджета и имя свойства (например, "text").
    // Загружает текущее значение из виджета. Если свойства нет,
    // значение будет пустой строкой.
    StringEditor(UIDocument& doc, WidgetID widgetId, const std::string& propertyName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
    {
        // Загружаем текущее значение.
        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            auto optVal = widget->getProperty(propertyName);
            if (optVal.has_value() && std::holds_alternative<std::string>(*optVal)) {
                m_currentValue = std::get<std::string>(*optVal);
            }
        }
    }

    // ── Текущее значение ─────────────────────────────────────
    [[nodiscard]] const std::string& value() const { return m_currentValue; }

    // ── Установка нового значения ────────────────────────────
    // Создаёт ChangePropertyCommand и выполняет её.
    // Если новое значение совпадает со старым, ничего не делает.
    void setValue(const std::string& newValue) {
        if (newValue == m_currentValue) return;

        // Создаём команду изменения свойства.
        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName, StateValue(newValue)
        );
        m_doc.history().execute(std::move(cmd));

        // Обновляем локальное значение.
        m_currentValue = newValue;
    }

    // ── Имя свойства ─────────────────────────────────────────
    [[nodiscard]] const std::string& propertyName() const { return m_propertyName; }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    std::string m_currentValue;
};

} // namespace MirUI