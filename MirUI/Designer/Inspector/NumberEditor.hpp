
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <string>
#include <cmath>

namespace MirUI {

class NumberEditor {
public:

    NumberEditor(UIDocument& doc,
                 WidgetID widgetId,
                 const std::string& propertyName,
                 double minValue = 0.0,
                 double maxValue = 1e9,
                 double step = 1.0)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
        , m_minValue(minValue)
        , m_maxValue(maxValue)
        , m_step(step)
        , m_isInteger(false)
    {

        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            auto optVal = widget->getProperty(propertyName);
            if (optVal.has_value()) {

                if (std::holds_alternative<int64_t>(*optVal)) {
                    m_currentValue = static_cast<double>(std::get<int64_t>(*optVal));
                    m_isInteger = true;
                } else if (std::holds_alternative<double>(*optVal)) {
                    m_currentValue = std::get<double>(*optVal);
                    m_isInteger = false;
                } else if (std::holds_alternative<bool>(*optVal)) {
                    m_currentValue = std::get<bool>(*optVal) ? 1.0 : 0.0;
                    m_isInteger = true;
                }
            }
        }

        m_currentValue = std::clamp(m_currentValue, m_minValue, m_maxValue);
    }

    [[nodiscard]] double value() const { return m_currentValue; }

    void setValue(double newValue) {

        if (m_isInteger) {
            newValue = std::round(newValue);
        }

        newValue = std::clamp(newValue, m_minValue, m_maxValue);

        if (std::abs(newValue - m_currentValue) < 1e-9) return;

        StateValue stateVal;
        if (m_isInteger) {
            stateVal = StateValue(static_cast<int64_t>(newValue));
        } else {
            stateVal = StateValue(newValue);
        }

        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName, stateVal
        );
        m_doc.history().execute(std::move(cmd));

        m_currentValue = newValue;
    }

    void increment() {
        setValue(m_currentValue + m_step);
    }

    void decrement() {
        setValue(m_currentValue - m_step);
    }

    [[nodiscard]] double minValue() const { return m_minValue; }
    [[nodiscard]] double maxValue() const { return m_maxValue; }
    [[nodiscard]] double step() const { return m_step; }
    [[nodiscard]] bool isInteger() const { return m_isInteger; }
    [[nodiscard]] const std::string& propertyName() const { return m_propertyName; }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    double      m_currentValue = 0.0;
    double      m_minValue;
    double      m_maxValue;
    double      m_step;
    bool        m_isInteger;
};

}