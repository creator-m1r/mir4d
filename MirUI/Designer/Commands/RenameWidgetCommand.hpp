
#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../Document/UIDocument.hpp"
#include <string>

namespace MirUI {

class RenameWidgetCommand : public ICommand {
public:

    RenameWidgetCommand(UIDocument& doc,
                        WidgetID widgetId,
                        const std::string& newName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_newName(newName)
        , m_oldName()
    {}

    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) {
            return false;
        }

        m_oldName = widget->name();

        if (m_oldName == m_newName) {
            return false;
        }

        widget->setProperty("name", StateValue(m_newName));

        m_doc.setModified(true);
        return true;
    }

    bool undo() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false;

        widget->setProperty("name", StateValue(m_oldName));

        m_doc.setModified(true);
        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Переименовать виджет в «" + m_newName + "»";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_newName;
    std::string m_oldName;
};

}