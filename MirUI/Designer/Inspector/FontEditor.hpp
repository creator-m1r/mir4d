
#pragma once

#include "../../Foundation/Typography/Font.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../Document/UIDocument.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <string>
#include <sstream>
#include <vector>

namespace MirUI {

class FontEditor {
public:

    FontEditor(UIDocument& doc, WidgetID widgetId, const std::string& propertyName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
    {

        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            auto value = widget->getProperty(propertyName);
            if (value.has_value() && std::holds_alternative<std::string>(*value)) {
                m_currentFont = deserializeFont(std::get<std::string>(*value));
            }
        }

        if (m_currentFont.family.empty()) {
            m_currentFont = Font("System", 14.0, FontWeight::Regular, FontStyle::Normal);
        }
    }

    [[nodiscard]] Font currentFont() const { return m_currentFont; }

    void setFont(const Font& newFont) {
        if (newFont == m_currentFont) return;

        std::string serialized = serializeFont(newFont);

        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName,
            StateValue(serialized)
        );

        m_doc.history().execute(std::move(cmd));

        m_currentFont = newFont;
    }

    bool showFontDialog() {

        static int step = 0;
        Font testFonts[] = {
            Font("System", 12.0, FontWeight::Regular, FontStyle::Normal),
            Font("System", 16.0, FontWeight::Bold, FontStyle::Italic),
            Font("Menlo",   14.0, FontWeight::Medium, FontStyle::Normal),
            Font("Georgia", 18.0, FontWeight::SemiBold, FontStyle::Normal),
        };
        setFont(testFonts[step % 4]);
        ++step;
        return true;
    }

    void resetToDefault() {
        setFont(Font("System", 14.0, FontWeight::Regular, FontStyle::Normal));
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    Font        m_currentFont;

    static std::string serializeFont(const Font& font) {
        return font.family + ";"
             + std::to_string(font.size) + ";"
             + std::to_string(static_cast<int>(font.weight)) + ";"
             + std::to_string(static_cast<int>(font.style));
    }

    static Font deserializeFont(const std::string& data) {
        Font result("System", 14.0, FontWeight::Regular, FontStyle::Normal);

        std::stringstream ss(data);
        std::string token;
        std::vector<std::string> parts;
        while (std::getline(ss, token, ';')) {
            parts.push_back(token);
        }

        if (parts.size() >= 1) result.family = parts[0];
        if (parts.size() >= 2) {
            try { result.size = std::stod(parts[1]); } catch (...) {}
        }
        if (parts.size() >= 3) {
            try {
                int w = std::stoi(parts[2]);
                result.weight = static_cast<FontWeight>(w);
            } catch (...) {}
        }
        if (parts.size() >= 4) {
            try {
                int s = std::stoi(parts[3]);
                result.style = static_cast<FontStyle>(s);
            } catch (...) {}
        }
        return result;
    }
};

}