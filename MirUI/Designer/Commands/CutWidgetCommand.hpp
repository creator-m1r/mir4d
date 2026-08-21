
#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../../Core/Widget/WidgetFactory.hpp"
#include "../Document/UIDocument.hpp"
#include "WidgetClipboard.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace MirUI {

class CutWidgetCommand : public ICommand {
public:

    CutWidgetCommand(UIDocument& doc, WidgetID widgetId)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_parentId()
        , m_oldIndex(0)
        , m_savedProperties()
    {}

    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false;

        Widget* parent = widget->parent();
        if (!parent) {

            return false;
        }

        m_parentId = parent->id();
        const auto& siblings = parent->children();
        for (size_t i = 0; i < siblings.size(); ++i) {
            if (siblings[i]->id() == m_widgetId) {
                m_oldIndex = i;
                break;
            }
        }

        m_savedProperties.clear();
        m_savedProperties["type"] = StateValue(static_cast<int64_t>(static_cast<int>(widget->type())));
        m_savedProperties["name"] = StateValue(widget->name());
        m_savedProperties["visible"] = StateValue(widget->isVisible());
        m_savedProperties["enabled"] = StateValue(widget->isEnabled());
        m_savedProperties["x"] = StateValue(widget->bounds().x);
        m_savedProperties["y"] = StateValue(widget->bounds().y);
        m_savedProperties["width"] = StateValue(widget->bounds().width);
        m_savedProperties["height"] = StateValue(widget->bounds().height);
        const auto& allProps = widget->allProperties();
        for (const auto& [key, val] : allProps) {
            if (m_savedProperties.find(key) == m_savedProperties.end()) {
                m_savedProperties[key] = val;
            }
        }

        WidgetClipboard::instance().setContent(widget->type(),
            std::unordered_map<std::string, StateValue>(m_savedProperties));

        parent->removeChild(m_widgetId);
        m_doc.widgetTree().unregisterWidget(m_widgetId);
        delete widget;

        m_doc.setModified(true);
        return true;
    }

    bool undo() override {

        if (m_savedProperties.empty()) return false;

        Widget* parent = m_doc.widgetTree().find(m_parentId);
        if (!parent) {

            parent = m_doc.widgetTree().root();
            if (!parent) return false;
            m_parentId = parent->id();
        }

        auto it = m_savedProperties.find("type");
        WidgetType type = WidgetType::Unknown;
        if (it != m_savedProperties.end() && std::holds_alternative<int64_t>(it->second)) {
            type = static_cast<WidgetType>(std::get<int64_t>(it->second));
        }
        auto widgetPtr = WidgetFactory::create(type);
        if (!widgetPtr) return false;

        Widget* newWidget = widgetPtr.get();

        for (const auto& [key, val] : m_savedProperties) {
            if (key != "type") {
                newWidget->setProperty(key, val);
            }
        }

        parent->addChild(widgetPtr.release());
        m_doc.widgetTree().registerWidget(newWidget);
        m_widgetId = newWidget->id();

        m_doc.setModified(true);
        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Вырезать виджет";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    WidgetID    m_parentId;
    size_t      m_oldIndex;
    std::unordered_map<std::string, StateValue> m_savedProperties;
};

}