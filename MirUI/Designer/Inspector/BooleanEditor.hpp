// MirUI/Designer/Inspector/BooleanEditor.hpp
// ✅ Редактор логического свойства (Boolean) — переключатель «Да / Нет».
//
// В инспекторе свойств логические значения (например, «Видимость»,
// «Доступность», «Нажатое состояние») отображаются в виде галочки
// или переключателя. BooleanEditor управляет этим значением:
// он хранит текущее состояние (true или false) и позволяет его изменить.
//
// При изменении значения редактор не меняет виджет напрямую.
// Вместо этого он создаёт команду ChangePropertyCommand и выполняет её
// через историю документа. Благодаря этому изменение можно отменить
// через Ctrl+Z, точно так же, как любое другое действие в редакторе.
//
// Сам BooleanEditor не рисует галочку — он только хранит значение
// и обрабатывает его изменение. Галочку отрисовывает платформенный
// рендерер (SwiftUI, WinUI), запрашивая текущее значение через value().
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

class BooleanEditor {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Принимает документ, ID виджета и имя свойства (например, "visible").
    // Загружает текущее значение из виджета. Если свойство не найдено,
    // считает его равным false.
    BooleanEditor(UIDocument& doc, WidgetID widgetId, const std::string& propertyName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
    {
        // Загружаем текущее значение.
        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            auto optVal = widget->getProperty(propertyName);
            if (optVal.has_value() && std::holds_alternative<bool>(*optVal)) {
                m_currentValue = std::get<bool>(*optVal);
            } else {
                m_currentValue = false;
            }
        } else {
            m_currentValue = false;
        }
    }

    // ── Текущее значение ─────────────────────────────────────
    [[nodiscard]] bool value() const { return m_currentValue; }

    // ── Установка нового значения ────────────────────────────
    // Создаёт ChangePropertyCommand и выполняет её.
    // Если новое значение совпадает с текущим, ничего не делает.
    void setValue(bool newValue) {
        if (newValue == m_currentValue) return;

        // Создаём команду изменения свойства.
        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName, StateValue(newValue)
        );
        // Выполняем — она попадёт в историю.
        m_doc.history().execute(std::move(cmd));

        // Обновляем локальное значение.
        m_currentValue = newValue;
    }

    // ── Переключение (toggle) ────────────────────────────────
    // Удобный метод: меняет значение на противоположное.
    void toggle() {
        setValue(!m_currentValue);
    }

    // ── Имя свойства ─────────────────────────────────────────
    [[nodiscard]] const std::string& propertyName() const { return m_propertyName; }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    bool        m_currentValue;
};

} // namespace MirUI