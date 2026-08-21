
#pragma once

#include "../Core/Rendering/Renderer.hpp"
#include "../Core/Widget/WidgetTree.hpp"
#include "../Core/Widget/Widget.hpp"
#include <iostream>
#include <string>

namespace MirUI {

class NullRenderer : public Renderer {
public:

    void beginFrame() override {

        m_widgetCount = 0;
        std::cout << "=== NullRenderer: начало кадра ===" << std::endl;
    }

    void render(WidgetTree& tree) override {

        Widget* root = tree.root();
        if (root) {
            std::cout << "Начинаем обход дерева виджетов..." << std::endl;
            renderWidgetRecursive(root, 0);
        } else {
            std::cout << "Дерево виджетов пустое — нечего рисовать." << std::endl;
        }
    }

    void endFrame() override {
        std::cout << "=== NullRenderer: конец кадра ===" << std::endl;
        std::cout << "Всего виджетов обработано: " << m_widgetCount << std::endl << std::endl;
    }

    [[nodiscard]] int getWidgetCount() const { return m_widgetCount; }

private:
    int m_widgetCount = 0;

    void renderWidgetRecursive(Widget* widget, int depth) {
        if (!widget) return;

        ++m_widgetCount;

        std::string indent(depth * 2, ' ');

        std::cout << indent
                  << "Виджет ID=" << widget->id().value()
                  << " | тип: " << widgetTypeToString(widget->type())
                  << " | visible=" << (widget->isVisible() ? "да" : "нет")
                  << " | bounds=(" << widget->bounds().x << ", " << widget->bounds().y
                  << ", " << widget->bounds().width << "x" << widget->bounds().height << ")"
                  << std::endl;

        for (Widget* child : widget->children()) {
            renderWidgetRecursive(child, depth + 1);
        }
    }

    static std::string widgetTypeToString(WidgetType type) {
        switch (type) {
            case WidgetType::Unknown:      return "Unknown";
            case WidgetType::Window:       return "Window";
            case WidgetType::Panel:        return "Panel";
            case WidgetType::Button:       return "Button";
            case WidgetType::Label:        return "Label";
            case WidgetType::Tree:         return "Tree";
            case WidgetType::PropertyGrid: return "PropertyGrid";
            case WidgetType::Ribbon:       return "Ribbon";
            case WidgetType::Toolbar:      return "Toolbar";
            case WidgetType::DockPanel:    return "DockPanel";
            case WidgetType::Viewport:     return "Viewport";
            case WidgetType::Timeline:     return "Timeline";
            default:                       return "???";
        }
    }
};

}