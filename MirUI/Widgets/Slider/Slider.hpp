
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include <string>
#include <algorithm>

namespace MirUI {

class Slider : public Widget {
public:

    explicit Slider(double minValue = 0.0, double maxValue = 100.0, double initialValue = 50.0)
        : Widget(WidgetType::Slider)
    {
        setProperty("value", StateValue(initialValue));
        setProperty("minValue", StateValue(minValue));
        setProperty("maxValue", StateValue(maxValue));
        setProperty("step", StateValue(0.0));
        setProperty("enabled", StateValue(true));
        setProperty("orientation", StateValue(std::string("horizontal")));

        setLayoutData(LayoutData::fixed(200, 20));
    }

    void setValue(double value) {
        double minVal = getMinValue();
        double maxVal = getMaxValue();
        value = std::clamp(value, minVal, maxVal);
        double st = getStep();
        if (st > 0.0) {

            value = minVal + std::round((value - minVal) / st) * st;
        }
        setProperty("value", StateValue(value));
    }

    [[nodiscard]] double getValue() const {
        auto val = getProperty("value");
        if (val.has_value() && std::holds_alternative<double>(*val)) {
            return std::get<double>(*val);
        }
        return 0.0;
    }

    void setMinValue(double minValue) {
        setProperty("minValue", StateValue(minValue));

        if (getValue() < minValue) setValue(minValue);
    }

    [[nodiscard]] double getMinValue() const {
        auto val = getProperty("minValue");
        if (val.has_value() && std::holds_alternative<double>(*val)) {
            return std::get<double>(*val);
        }
        return 0.0;
    }

    void setMaxValue(double maxValue) {
        setProperty("maxValue", StateValue(maxValue));
        if (getValue() > maxValue) setValue(maxValue);
    }

    [[nodiscard]] double getMaxValue() const {
        auto val = getProperty("maxValue");
        if (val.has_value() && std::holds_alternative<double>(*val)) {
            return std::get<double>(*val);
        }
        return 100.0;
    }

    void setStep(double step) {
        setProperty("step", StateValue(step));
    }

    [[nodiscard]] double getStep() const {
        auto val = getProperty("step");
        if (val.has_value() && std::holds_alternative<double>(*val)) {
            return std::get<double>(*val);
        }
        return 0.0;
    }

    void setOrientation(const std::string& orientation) {
        setProperty("orientation", StateValue(orientation));
    }

    [[nodiscard]] std::string getOrientation() const {
        auto val = getProperty("orientation");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            return std::get<std::string>(*val);
        }
        return "horizontal";
    }

    bool setProperty(const std::string& name, const StateValue& value) override {
        if (name == "value" && std::holds_alternative<double>(value)) {
            double v = std::get<double>(value);
            v = std::clamp(v, getMinValue(), getMaxValue());
            m_properties[name] = StateValue(v);
            return true;
        }
        if (name == "minValue" && std::holds_alternative<double>(value)) {
            m_properties[name] = value;

            if (getValue() < std::get<double>(value)) setValue(std::get<double>(value));
            return true;
        }
        if (name == "maxValue" && std::holds_alternative<double>(value)) {
            m_properties[name] = value;
            if (getValue() > std::get<double>(value)) setValue(std::get<double>(value));
            return true;
        }
        if (name == "step" && std::holds_alternative<double>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "orientation" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        return Widget::setProperty(name, value);
    }

    std::optional<StateValue> getProperty(const std::string& name) const override {
        if (name == "value" || name == "minValue" || name == "maxValue" ||
            name == "step" || name == "orientation") {
            auto it = m_properties.find(name);
            if (it != m_properties.end()) return it->second;
        }
        return Widget::getProperty(name);
    }
};

}