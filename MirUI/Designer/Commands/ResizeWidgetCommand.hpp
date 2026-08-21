// MirUI/Designer/Commands/ResizeWidgetCommand.hpp
// ↔️ Команда «Изменить размер виджета» — запоминает старые и новые
// границы (bounds) виджета, чтобы можно было откатить изменение.
// Вызывается, когда пользователь тянет за уголки на холсте редактора.
// Поддерживает Undo (Ctrl+Z) и Redo (Ctrl+Shift+Z).
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Commands/CommandHistory.hpp" // ICommand
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Layout/Rect.hpp"
#include "../Document/UIDocument.hpp"

namespace MirUI {

class ResizeWidgetCommand : public ICommand {
public:
    // Конструктор принимает документ, ID изменяемого виджета и новые границы.
    ResizeWidgetCommand(UIDocument& doc,
                        WidgetID widgetId,
                        const Rect& newBounds)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_newBounds(newBounds)
        , m_oldBounds() // будет заполнен при execute
    {}

    // ── execute() — применить новый размер ───────────────────
    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) {
            return false; // виджет не найден
        }

        // Сохраняем старые границы, чтобы можно было откатить.
        m_oldBounds = widget->bounds();

        // Применяем новый размер и положение.
        widget->setBounds(m_newBounds);

        // Помечаем документ как изменённый.
        m_doc.setModified(true);

        return true;
    }

    // ── undo() — вернуть старый размер ───────────────────────
    bool undo() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) {
            return false;
        }

        // Восстанавливаем сохранённые старые границы.
        widget->setBounds(m_oldBounds);

        m_doc.setModified(true);
        return true;
    }

    // ── Описание для отображения в истории ──────────────────
    [[nodiscard]] std::string description() const override {
        return "Изменить размер виджета";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;      // чей размер меняем
    Rect        m_newBounds;     // на что меняем
    Rect        m_oldBounds;     // что было до изменения (для undo)
};

} // namespace MirUI