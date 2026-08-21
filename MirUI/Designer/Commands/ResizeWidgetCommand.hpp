
#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Layout/Rect.hpp"
#include "../Document/UIDocument.hpp"

namespace MirUI {

class ResizeWidgetCommand : public ICommand {
public:

    ResizeWidgetCommand(UIDocument& doc,
                        WidgetID widgetId,
                        const Rect& newBounds)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_newBounds(newBounds)
        , m_oldBounds()
    {}

    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) {
            return false;
        }

        m_oldBounds = widget->bounds();

        widget->setBounds(m_newBounds);

        m_doc.setModified(true);

        return true;
    }

    bool undo() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) {
            return false;
        }

        widget->setBounds(m_oldBounds);

        m_doc.setModified(true);
        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Изменить размер виджета";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    Rect        m_newBounds;
    Rect        m_oldBounds;
};

}