
#pragma once

#include "../../Core/State/StateValue.hpp"
#include "../Document/UIDocument.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <optional>

namespace MirUI {

class EnumEditor {
public:

    EnumEditor(UIDocument& doc,
               WidgetID widgetId,
               const std::string& propertyName,
               const std::vector<std::string>& possibleValues)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
        , m_possibleValues(possibleValues)
    {

        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            auto value = widget->getProperty(propertyName);
            if (value.has_value() && std::holds_alternative<std::string>(*value)) {
                m_currentValue = std::get<std::string>(*value);
            }
        }

        if (!m_possibleValues.empty()) {
            if (!isValueValid(m_currentValue)) {
                m_currentValue = m_possibleValues.front();

            }
        }
    }

    [[nodiscard]] const std::string& currentValue() const {
        return m_currentValue;
    }

    [[nodiscard]] const std::vector<std::string>& possibleValues() const {
        return m_possibleValues;
    }

    void setValue(const std::string& newValue) {
        if (newValue == m_currentValue) return;

        if (!isValueValid(newValue)) {
            return;
        }

        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName,
            StateValue(newValue)
        );

        m_doc.history().execute(std::move(cmd));

        m_currentValue = newValue;
    }

    [[nodiscard]] int currentIndex() const {
        auto it = std::find(m_possibleValues.begin(), m_possibleValues.end(), m_currentValue);
        if (it != m_possibleValues.end()) {
            return static_cast<int>(std::distance(m_possibleValues.begin(), it));
        }
        return 0;
    }

    void resetToDefault() {
        if (!m_possibleValues.empty()) {
            setValue(m_possibleValues.front());
        }
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    std::vector<std::string> m_possibleValues;
    std::string m_currentValue;

    [[nodiscard]] bool isValueValid(const std::string& value) const {
        return std::find(m_possibleValues.begin(), m_possibleValues.end(), value)
               != m_possibleValues.end();
    }
};

}