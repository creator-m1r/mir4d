
#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../Document/UIDocument.hpp"
#include <string>
#include <optional>

namespace MirUI {

class ChangePropertyCommand : public ICommand {
public:

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

    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) {
            return false;
        }

        m_oldValue = widget->getProperty(m_propertyName);

        if (!widget->setProperty(m_propertyName, m_newValue)) {
            return false;
        }

        m_doc.setModified(true);
        return true;
    }

    bool undo() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false;

        if (!m_oldValue.has_value()) {

            return true;
        }

        widget->setProperty(m_propertyName, *m_oldValue);
        m_doc.setModified(true);
        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Изменить свойство «" + m_propertyName + "»";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    StateValue  m_newValue;
    std::optional<StateValue> m_oldValue;
};

}