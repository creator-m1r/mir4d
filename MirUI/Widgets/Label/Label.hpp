
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include <string>

namespace MirUI {

enum class TextAlignment {
    Left,
    Center,
    Right
};

class Label : public Widget {
public:
    explicit Label(const std::string& text = "")
        : Widget(WidgetType::Label)
        , m_text(text)
        , m_alignment(TextAlignment::Left)
    {}

    void setText(const std::string& text) { m_text = text; }
    [[nodiscard]] const std::string& text() const { return m_text; }

    void setAlignment(TextAlignment alignment) { m_alignment = alignment; }
    [[nodiscard]] TextAlignment alignment() const { return m_alignment; }

private:
    std::string m_text;
    TextAlignment m_alignment = TextAlignment::Left;
};

}