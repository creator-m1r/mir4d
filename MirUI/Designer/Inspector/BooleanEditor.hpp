
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <string>

namespace MirUI {

class BooleanEditor {
public:

    BooleanEditor(UIDocument& doc, WidgetID widgetId, const std::string& propertyName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
    {

        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            auto optVal = widget->getProperty(propertyName);
            if (optVal.has_value() && std::holds_alternative<bool>(*optVal)) {
                m_currentValue = std::get<bool>(*optVal);
            } else {
                m_currentValue = false;
            }
        } else {
            m_currentValue = false;
        }
    }

    [[nodiscard]] bool value() const { return m_currentValue; }

    void setValue(bool newValue) {
        if (newValue == m_currentValue) return;

        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName, StateValue(newValue)
        );

        m_doc.history().execute(std::move(cmd));

        m_currentValue = newValue;
    }

    void toggle() {
        setValue(!m_currentValue);
    }

    [[nodiscard]] const std::string& propertyName() const { return m_propertyName; }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    bool        m_currentValue;
};

}