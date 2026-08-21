
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include <string>

namespace MirUI {

class CheckBox : public Widget {
public:

    explicit CheckBox(const std::string& text = "Флажок")
        : Widget(WidgetType::CheckBox)
    {

        setProperty("text", StateValue(text));
        setProperty("checked", StateValue(false));
        setProperty("enabled", StateValue(true));

        setLayoutData(LayoutData::fit());
    }

    void setText(const std::string& text) {
        setProperty("text", StateValue(text));
    }
    [[nodiscard]] std::string getText() const {
        auto val = getProperty("text");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            return std::get<std::string>(*val);
        }
        return "";
    }

    void setChecked(bool checked) {
        setProperty("checked", StateValue(checked));
    }
    [[nodiscard]] bool isChecked() const {
        auto val = getProperty("checked");
        if (val.has_value() && std::holds_alternative<bool>(*val)) {
            return std::get<bool>(*val);
        }
        return false;
    }

    void toggle() {
        setChecked(!isChecked());
    }

    bool setProperty(const std::string& name, const StateValue& value) override {

        if (name == "checked" && std::holds_alternative<bool>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "text" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        return Widget::setProperty(name, value);
    }

    std::optional<StateValue> getProperty(const std::string& name) const override {
        if (name == "checked" || name == "text") {
            auto it = m_properties.find(name);
            if (it != m_properties.end()) return it->second;
        }
        return Widget::getProperty(name);
    }
};

}