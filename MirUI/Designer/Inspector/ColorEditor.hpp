
#pragma once

#include "../../Foundation/Color/Color.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../Document/UIDocument.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <functional>

namespace MirUI {

class ColorEditor {
public:

    ColorEditor(UIDocument& doc, WidgetID widgetId, const std::string& propertyName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
    {

        auto value = doc.widgetTree().find(widgetId)->getProperty(propertyName);
        if (value.has_value() && std::holds_alternative<std::string>(*value)) {

            m_currentColor = Color::fromHex(std::get<std::string>(*value));
        } else {

            m_currentColor = Color::transparent();
        }
    }

    [[nodiscard]] Color color() const { return m_currentColor; }

    void setColor(const Color& newColor) {
        if (newColor == m_currentColor) return;

        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName,
            StateValue(newColor.toHex())
        );

        m_doc.history().execute(std::move(cmd));

        m_currentColor = newColor;
    }

    bool showColorDialog() {

        static int step = 0;
        Color testColors[] = {
            Color::rgb(1.0f, 0.0f, 0.0f),
            Color::rgb(0.0f, 1.0f, 0.0f),
            Color::rgb(0.0f, 0.0f, 1.0f),
            Color::white(),
        };
        setColor(testColors[step % 4]);
        ++step;
        return true;
    }

    void resetToDefault() {
        setColor(Color::transparent());
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    Color       m_currentColor;
};

}