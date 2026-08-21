
#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../../Core/Widget/WidgetFactory.hpp"
#include "../Document/UIDocument.hpp"
#include "WidgetClipboard.hpp"
#include <memory>
#include <string>

namespace MirUI {

class PasteWidgetCommand : public ICommand {
public:

    PasteWidgetCommand(UIDocument& doc, WidgetID parentId = WidgetID{})
        : m_doc(doc)
        , m_parentId(parentId)
        , m_createdId()
    {}

    bool execute() override {
        WidgetClipboard& clipboard = WidgetClipboard::instance();
        if (!clipboard.hasContent()) {
            return false;
        }

        Widget* parent = m_doc.widgetTree().find(m_parentId);
        if (!parent) {
            parent = m_doc.widgetTree().root();
        }
        if (!parent) {
            return false;
        }
        m_parentId = parent->id();

        WidgetType type = clipboard.type();
        auto widgetPtr = WidgetFactory::create(type);
        if (!widgetPtr) {
            return false;
        }
        Widget* newWidget = widgetPtr.get();

        const auto& props = clipboard.properties();
        for (const auto& [key, val] : props) {
            newWidget->setProperty(key, val);
        }

        Rect bounds = newWidget->bounds();
        bounds.x += 20.0;
        bounds.y += 20.0;
        newWidget->setBounds(bounds);

        parent->addChild(widgetPtr.release());
        m_createdId = newWidget->id();

        m_doc.widgetTree().registerWidget(newWidget);

        m_doc.setModified(true);
        return true;
    }

    bool undo() override {
        if (m_createdId.value() == 0) return false;

        Widget* parent = m_doc.widgetTree().find(m_parentId);
        if (!parent) return false;

        Widget* child = nullptr;
        for (Widget* c : parent->children()) {
            if (c->id() == m_createdId) {
                child = c;
                break;
            }
        }
        if (!child) return false;

        parent->removeChild(m_createdId);
        m_doc.widgetTree().unregisterWidget(m_createdId);
        delete child;
        m_createdId = WidgetID{};

        m_doc.setModified(true);
        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Вставить виджет из буфера";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_parentId;
    WidgetID    m_createdId;
};

}