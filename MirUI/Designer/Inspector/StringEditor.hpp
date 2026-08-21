
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <string>

namespace MirUI {

class StringEditor {
public:

    StringEditor(UIDocument& doc, WidgetID widgetId, const std::string& propertyName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
    {

        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            auto optVal = widget->getProperty(propertyName);
            if (optVal.has_value() && std::holds_alternative<std::string>(*optVal)) {
                m_currentValue = std::get<std::string>(*optVal);
            }
        }
    }

    [[nodiscard]] const std::string& value() const { return m_currentValue; }

    void setValue(const std::string& newValue) {
        if (newValue == m_currentValue) return;

        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName, StateValue(newValue)
        );
        m_doc.history().execute(std::move(cmd));

        m_currentValue = newValue;
    }

    [[nodiscard]] const std::string& propertyName() const { return m_propertyName; }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    std::string m_currentValue;
};

}