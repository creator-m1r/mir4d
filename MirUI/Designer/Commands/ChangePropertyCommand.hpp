// MirUI/Designer/Commands/ChangePropertyCommand.hpp
// ✏️ Команда «Изменить свойство виджета» — универсальный способ менять
// любое свойство любого виджета (текст, цвет, шрифт, видимость и т.д.)
// через единый интерфейс. Используется инспектором свойств (PropertyGrid).
// Поддерживает Undo (Ctrl+Z) и Redo (Ctrl+Shift+Z).
//
// 📦 Работает с абстрактным методом Widget::setProperty(name, value),
// который должен быть реализован каждым конкретным виджетом.
// Если виджет не знает такое свойство — команда ничего не делает.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Commands/CommandHistory.hpp" // ICommand
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../Document/UIDocument.hpp"
#include <string>
#include <optional>

namespace MirUI {

class ChangePropertyCommand : public ICommand {
public:
    // Конструктор:
    //   doc      — документ, в котором находится виджет
    //   widgetId — ID виджета, чьё свойство меняем
    //   propertyName — имя свойства (например, "text", "color", "visible")
    //   newValue — новое значение
    ChangePropertyCommand(UIDocument& doc,
                          WidgetID widgetId,
                          std::string propertyName,
                          StateValue newValue)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(std::move(propertyName))
        , m_newValue(std::move(newValue))
        , m_oldValue()
    {}

    // ── execute() — установить новое значение ────────────────
    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) {
            return false; // виджет не найден
        }

        // Сохраняем старое значение (если оно есть), чтобы можно было откатить.
        // Если виджет не поддерживает это свойство, getProperty вернёт std::nullopt.
        m_oldValue = widget->getProperty(m_propertyName);

        // Пытаемся установить новое значение.
        if (!widget->setProperty(m_propertyName, m_newValue)) {
            return false; // свойство не найдено или не может быть изменено
        }

        m_doc.setModified(true);
        return true;
    }

    // ── undo() — вернуть старое значение ─────────────────────
    bool undo() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false;

        // Если старое значение отсутствовало (свойство не было задано),
        // то восстанавливать нечего — просто ничего не делаем.
        if (!m_oldValue.has_value()) {
            // Свойство было изменено с "не задано" на новое.
            // При undo нужно удалить это свойство (установить std::nullopt).
            // У нас в setProperty нет такой возможности, поэтому пока пропускаем.
            // В будущем можно добавить метод removeProperty.
            return true;
        }

        // Восстанавливаем старое значение.
        widget->setProperty(m_propertyName, *m_oldValue);
        m_doc.setModified(true);
        return true;
    }

    // ── Описание для истории ─────────────────────────────────
    [[nodiscard]] std::string description() const override {
        return "Изменить свойство «" + m_propertyName + "»";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    StateValue  m_newValue;
    std::optional<StateValue> m_oldValue; // std::nullopt если свойства не было
};

} // namespace MirUI