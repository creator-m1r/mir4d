
#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../Document/UIDocument.hpp"
#include "WidgetClipboard.hpp"
#include <memory>
#include <string>

namespace MirUI {

class CopyWidgetCommand : public ICommand {
public:

    CopyWidgetCommand(UIDocument& doc, WidgetID widgetId)
        : m_doc(doc)
        , m_widgetId(widgetId)
    {}

    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false;

        std::unordered_map<std::string, StateValue> props;

        props["type"] = StateValue(static_cast<int64_t>(static_cast<int>(widget->type())));
        props["name"] = StateValue(widget->name());
        props["visible"] = StateValue(widget->isVisible());
        props["enabled"] = StateValue(widget->isEnabled());

        props["x"] = StateValue(widget->bounds().x);
        props["y"] = StateValue(widget->bounds().y);
        props["width"] = StateValue(widget->bounds().width);
        props["height"] = StateValue(widget->bounds().height);

        const auto& allProps = widget->allProperties();
        for (const auto& [key, val] : allProps) {

            if (key == "type" || key == "name" || key == "visible" || key == "enabled" ||
                key == "x" || key == "y" || key == "width" || key == "height") {
                continue;
            }
            props[key] = val;
        }

        WidgetClipboard::instance().setContent(widget->type(), std::move(props));

        return true;
    }

    bool undo() override {

        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Копировать виджет";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
};

}