
#pragma once

#include "../../Core/Widget/WidgetType.hpp"
#include "../../Core/State/StateValue.hpp"
#include <unordered_map>
#include <string>

namespace MirUI {

class WidgetClipboard {
public:

    static WidgetClipboard& instance() {
        static WidgetClipboard s_clipboard;
        return s_clipboard;
    }

    void setContent(WidgetType type, std::unordered_map<std::string, StateValue> properties) {
        m_type = type;
        m_properties = std::move(properties);
        m_hasContent = true;
    }

    [[nodiscard]] bool hasContent() const {
        return m_hasContent;
    }

    [[nodiscard]] WidgetType type() const {
        return m_type;
    }

    [[nodiscard]] const std::unordered_map<std::string, StateValue>& properties() const {
        return m_properties;
    }

    void clear() {
        m_type = WidgetType::Unknown;
        m_properties.clear();
        m_hasContent = false;
    }

private:

    WidgetClipboard() = default;

    WidgetType m_type = WidgetType::Unknown;
    std::unordered_map<std::string, StateValue> m_properties;
    bool m_hasContent = false;
};

}