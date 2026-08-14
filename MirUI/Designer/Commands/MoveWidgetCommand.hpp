// MirUI/Designer/Commands/MoveWidgetCommand.hpp
// 🖱️ Команда «Переместить виджет» — изменяет родителя и/или позицию виджета.
// Используется при drag & drop в редакторе. Поддерживает Undo/Redo.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../Document/UIDocument.hpp"

namespace MirUI {

class MoveWidgetCommand : public ICommand {
public:
    // Переместить виджет widgetId в нового родителя newParentId.
    // Если newParentId совпадает с текущим родителем, только меняем порядок (пока не реализовано).
    MoveWidgetCommand(UIDocument& doc, WidgetID widgetId, WidgetID newParentId)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_newParentId(newParentId)
        , m_oldParentId()
        , m_oldIndex(0)
    {}

    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false;

        Widget* oldParent = widget->parent();
        if (!oldParent) return false; // нельзя перемещать корень

        Widget* newParent = m_doc.widgetTree().find(m_newParentId);
        if (!newParent) return false;

        // Запоминаем старого родителя и позицию.
        m_oldParentId = oldParent->id();
        const auto& siblings = oldParent->children();
        for (size_t i = 0; i < siblings.size(); ++i) {
            if (siblings[i]->id() == m_widgetId) {
                m_oldIndex = i;
                break;
            }
        }

        // Отсоединяем от старого родителя (не удаляя).
        oldParent->removeChild(m_widgetId);

        // Присоединяем к новому родителю (в конец).
        newParent->addChild(widget);

        m_doc.setModified(true);
        return true;
    }

    bool undo() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false;

        Widget* currentParent = widget->parent();
        if (currentParent) {
            currentParent->removeChild(m_widgetId);
        }

        Widget* oldParent = m_doc.widgetTree().find(m_oldParentId);
        if (!oldParent) return false;

        oldParent->addChild(widget);
        // TODO: восстановить позицию m_oldIndex, когда появится insertChild.

        m_doc.setModified(true);
        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Переместить виджет";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    WidgetID    m_newParentId;
    WidgetID    m_oldParentId;
    size_t      m_oldIndex;
};

} // namespace MirUI