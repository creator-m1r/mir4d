
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>

namespace MirUI {

class ComboBox : public Widget {
public:

    explicit ComboBox(const std::vector<std::string>& initialItems = {})
        : Widget(WidgetType::ComboBox)
    {

        setItems(initialItems);
        setProperty("selectedIndex", StateValue(static_cast<int64_t>(initialItems.empty() ? -1 : 0)));
        setProperty("enabled", StateValue(true));

        setLayoutData(LayoutData::fixed(200, 28));
    }

    void setItems(const std::vector<std::string>& items) {

        std::string joined;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) joined += "|";

            std::string escaped = items[i];
            size_t pos = 0;
            while ((pos = escaped.find('|', pos)) != std::string::npos) {
                escaped.insert(pos, "\\");
                pos += 2;
            }
            joined += escaped;
        }
        setProperty("items", StateValue(joined));
    }

    [[nodiscard]] std::vector<std::string> getItems() const {
        auto val = getProperty("items");
        if (!val.has_value() || !std::holds_alternative<std::string>(*val)) {
            return {};
        }
        return splitEscaped(std::get<std::string>(*val), '|');
    }

    void setSelectedIndex(int64_t index) {
        setProperty("selectedIndex", StateValue(index));
    }

    [[nodiscard]] int64_t selectedIndex() const {
        auto val = getProperty("selectedIndex");
        if (val.has_value() && std::holds_alternative<int64_t>(*val)) {
            return std::get<int64_t>(*val);
        }
        return -1;
    }

    [[nodiscard]] std::string selectedText() const {
        int64_t idx = selectedIndex();
        auto items = getItems();
        if (idx >= 0 && static_cast<size_t>(idx) < items.size()) {
            return items[static_cast<size_t>(idx)];
        }
        return "";
    }

    bool setProperty(const std::string& name, const StateValue& value) override {
        if (name == "items" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "selectedIndex" && std::holds_alternative<int64_t>(value)) {
            m_properties[name] = value;
            return true;
        }
        return Widget::setProperty(name, value);
    }

    std::optional<StateValue> getProperty(const std::string& name) const override {
        if (name == "items" || name == "selectedIndex") {
            auto it = m_properties.find(name);
            if (it != m_properties.end()) return it->second;
        }
        return Widget::getProperty(name);
    }

private:

    static std::vector<std::string> splitEscaped(const std::string& str, char delimiter) {
        std::vector<std::string> result;
        std::string current;
        bool escape = false;
        for (char ch : str) {
            if (escape) {
                current += ch;
                escape = false;
            } else if (ch == '\\') {
                escape = true;
            } else if (ch == delimiter) {
                result.push_back(current);
                current.clear();
            } else {
                current += ch;
            }
        }
        result.push_back(current);
        return result;
    }
};

}