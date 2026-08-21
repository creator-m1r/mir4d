
#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../Document/UIDocument.hpp"
#include <memory>
#include <string>
#include <stdexcept>

namespace MirUI {

class DeleteWidgetCommand : public ICommand {
public:

    DeleteWidgetCommand(UIDocument& doc, WidgetID widgetId)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_parentId()
        , m_oldIndex(0)
        , m_savedWidget(nullptr)
    {}

    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false;

        Widget* parent = widget->parent();
        if (!parent) {

            return false;
        }

        m_parentId = parent->id();
        const auto& children = parent->children();
        for (size_t i = 0; i < children.size(); ++i) {
            if (children[i]->id() == m_widgetId) {
                m_oldIndex = i;
                break;
            }
        }

        m_savedWidget = widget;

        parent->removeChild(m_widgetId);

        m_doc.selection().deselect(m_widgetId);

        m_doc.setModified(true);
        return true;
    }

    bool undo() override {
        if (!m_savedWidget) return false;

        Widget* parent = m_doc.widgetTree().find(m_parentId);
        if (!parent) {

            return false;
        }

        parent->addChild(m_savedWidget);

        m_doc.selection().addToSelection(m_widgetId);
        m_doc.setModified(true);

        m_savedWidget = nullptr;

        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Удалить виджет";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    WidgetID    m_parentId;
    size_t      m_oldIndex;
    Widget*     m_savedWidget;
};

}