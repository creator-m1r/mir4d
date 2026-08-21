
#pragma once

#include "../../Core/Layout/LayoutEngine.hpp"
#include "../../Core/Layout/LayoutNode.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <memory>
#include <vector>
#include <string>

namespace MirUI {

class LayoutManager {
public:

    explicit LayoutManager(WidgetTree& tree)
        : m_tree(tree)
    {

        m_layoutEngine = std::make_unique<LayoutEngine>();
    }

    void dock(WidgetID widgetToDock, WidgetID targetWidget, DockPosition position) {

        Widget* source = m_tree.find(widgetToDock);
        Widget* target = m_tree.find(targetWidget);
        if (!source || !target) return;

        Widget* oldParent = source->parent();
        if (oldParent) {
            oldParent->removeChild(widgetToDock);
        }

        target->addChild(source);
        
    }

    void undock(WidgetID widgetID) {
        Widget* widget = m_tree.find(widgetID);
        if (!widget) return;

        Widget* parent = widget->parent();
        if (parent) {
            parent->removeChild(widgetID);

        }
    }

    void splitHorizontal(WidgetID containerID, WidgetID newWidget) {

    }

    void splitVertical(WidgetID containerID, WidgetID newWidget) {

    }

    void resize(WidgetID widgetID, const Size& newSize) {
        Widget* widget = m_tree.find(widgetID);
        if (!widget) return;

        Rect bounds = widget->bounds();
        bounds.width  = newSize.width;
        bounds.height = newSize.height;
        widget->setBounds(bounds);

        m_layoutEngine->layout(m_tree);
    }

    void floatPanel(WidgetID panelID) {

        undock(panelID);

    }

    void restore(const std::string& workspaceName) {

        m_layoutEngine->layout(m_tree);
    }

    void setLayoutEngine(std::unique_ptr<LayoutEngine> engine) {
        m_layoutEngine = std::move(engine);
    }

    [[nodiscard]] LayoutEngine* layoutEngine() const {
        return m_layoutEngine.get();
    }

private:
    WidgetTree& m_tree;
    std::unique_ptr<LayoutEngine> m_layoutEngine;
};

}