
#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../Document/UIDocument.hpp"

namespace MirUI {

class MoveWidgetCommand : public ICommand {
public:

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
        if (!oldParent) return false;

        Widget* newParent = m_doc.widgetTree().find(m_newParentId);
        if (!newParent) return false;

        m_oldParentId = oldParent->id();
        const auto& siblings = oldParent->children();
        for (size_t i = 0; i < siblings.size(); ++i) {
            if (siblings[i]->id() == m_widgetId) {
                m_oldIndex = i;
                break;
            }
        }

        oldParent->removeChild(m_widgetId);

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

}