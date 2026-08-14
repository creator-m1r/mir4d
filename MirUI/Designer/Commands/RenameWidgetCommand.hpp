// MirUI/Designer/Commands/RenameWidgetCommand.hpp
// ✏️ Команда «Переименовать виджет» — изменяет отображаемое имя виджета.
// Используется, когда пользователь дважды кликает по названию виджета
// в инспекторе или дереве и вводит новое имя.
// Поддерживает Undo (Ctrl+Z) и Redo (Ctrl+Shift+Z).
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Commands/CommandHistory.hpp" // ICommand
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../Document/UIDocument.hpp"
#include <string>

namespace MirUI {

class RenameWidgetCommand : public ICommand {
public:
    // Конструктор:
    //   doc      — документ, в котором находится виджет
    //   widgetId — ID переименовываемого виджета
    //   newName  — новое имя
    RenameWidgetCommand(UIDocument& doc,
                        WidgetID widgetId,
                        const std::string& newName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_newName(newName)
        , m_oldName()
    {}

    // ── execute() — установить новое имя ─────────────────────
    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) {
            return false; // виджет не найден
        }

        // Сохраняем старое имя, чтобы можно было откатить.
        m_oldName = widget->name();

        // Если новое имя совпадает со старым, ничего не делаем.
        if (m_oldName == m_newName) {
            return false;
        }

        // Устанавливаем новое имя через универсальный setProperty.
        widget->setProperty("name", StateValue(m_newName));

        m_doc.setModified(true);
        return true;
    }

    // ── undo() — вернуть старое имя ──────────────────────────
    bool undo() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false;

        // Восстанавливаем старое имя.
        widget->setProperty("name", StateValue(m_oldName));

        m_doc.setModified(true);
        return true;
    }

    // ── Описание для истории ─────────────────────────────────
    [[nodiscard]] std::string description() const override {
        return "Переименовать виджет в «" + m_newName + "»";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_newName;
    std::string m_oldName; // сохранённое старое имя для отката
};

} // namespace MirUI