
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include <string>
#include <cstdint>

namespace MirUI {

class TextField : public Widget {
public:

    explicit TextField(const std::string& initialText = "",
                       const std::string& placeholder = "Введите текст...")
        : Widget(WidgetType::TextField)
    {

        setProperty("text", StateValue(initialText));
        setProperty("placeholder", StateValue(placeholder));
        setProperty("enabled", StateValue(true));
        setProperty("readOnly", StateValue(false));
        setProperty("maxLength", StateValue(static_cast<int64_t>(0)));
        setProperty("textAlignment", StateValue(std::string("Left")));

        setLayoutData(LayoutData::fixed(200, 28));
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

    void setPlaceholder(const std::string& placeholder) {
        setProperty("placeholder", StateValue(placeholder));
    }
    [[nodiscard]] std::string getPlaceholder() const {
        auto val = getProperty("placeholder");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            return std::get<std::string>(*val);
        }
        return "";
    }

    void setReadOnly(bool readOnly) {
        setProperty("readOnly", StateValue(readOnly));
    }
    [[nodiscard]] bool isReadOnly() const {
        auto val = getProperty("readOnly");
        if (val.has_value() && std::holds_alternative<bool>(*val)) {
            return std::get<bool>(*val);
        }
        return false;
    }

    void setMaxLength(int64_t maxLen) {
        setProperty("maxLength", StateValue(maxLen));
    }
    [[nodiscard]] int64_t maxLength() const {
        auto val = getProperty("maxLength");
        if (val.has_value() && std::holds_alternative<int64_t>(*val)) {
            return std::get<int64_t>(*val);
        }
        return 0;
    }

    void setTextAlignment(const std::string& alignment) {
        setProperty("textAlignment", StateValue(alignment));
    }
    [[nodiscard]] std::string getTextAlignment() const {
        auto val = getProperty("textAlignment");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            return std::get<std::string>(*val);
        }
        return "Left";
    }

    bool setProperty(const std::string& name, const StateValue& value) override {
        if (name == "text" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "placeholder" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "readOnly" && std::holds_alternative<bool>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "maxLength" && std::holds_alternative<int64_t>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "textAlignment" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        return Widget::setProperty(name, value);
    }

    std::optional<StateValue> getProperty(const std::string& name) const override {
        if (name == "text" || name == "placeholder" ||
            name == "readOnly" || name == "maxLength" ||
            name == "textAlignment") {
            auto it = m_properties.find(name);
            if (it != m_properties.end()) return it->second;
        }
        return Widget::getProperty(name);
    }
};

}