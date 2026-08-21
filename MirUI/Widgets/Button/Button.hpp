
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include "../../Foundation/Icons/IconID.hpp"
#include "../../Core/Commands/CommandID.hpp"
#include <string>
#include <functional>

namespace MirUI {

class Button : public Widget {
public:

    explicit Button(const std::string& text = "")
        : Widget(WidgetType::Button)
        , m_text(text)
    {}

    void setText(const std::string& text) { m_text = text; }

    [[nodiscard]] const std::string& text() const { return m_text; }

    void setIcon(const IconID& icon) { m_icon = icon; }

    [[nodiscard]] const IconID& icon() const { return m_icon; }

    void setCommand(const CommandID& command) { m_command = command; }

    [[nodiscard]] const CommandID& command() const { return m_command; }

    void setToggle(bool isToggle) { m_isToggle = isToggle; }
    [[nodiscard]] bool isToggle() const { return m_isToggle; }

    void setChecked(bool checked) { m_checked = checked; }
    [[nodiscard]] bool isChecked() const { return m_checked; }

    void click() {
        if (m_onClick) {
            m_onClick(*this);
        }
    }

    void setOnClick(std::function<void(Button&)> callback) {
        m_onClick = std::move(callback);
    }

private:

    std::string m_text;
    IconID      m_icon;
    CommandID   m_command;
    bool        m_isToggle = false;
    bool        m_checked = false;
    std::function<void(Button&)> m_onClick;
};

}