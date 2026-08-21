
#pragma once

#include "../Widget/WidgetTree.hpp"
#include "../Widget/WidgetType.hpp"
#include "Renderer.hpp"

namespace MirUI {

class WidgetRenderer : public Renderer {
public:
    virtual ~WidgetRenderer() = default;

    void render(WidgetTree& tree) override {
        Widget* root = tree.root();
        if (root) {
            renderWidgetRecursive(root);
        }
    }

protected:
    virtual void renderButton(Widget& widget)       { (void)widget; }
    virtual void renderLabel(Widget& widget)        { (void)widget; }
    virtual void renderToolbar(Widget& widget)      { (void)widget; }
    virtual void renderTree(Widget& widget)         { (void)widget; }
    virtual void renderPropertyGrid(Widget& widget) { (void)widget; }
    virtual void renderViewport(Widget& widget)     { (void)widget; }
    virtual void renderContainer(Widget& widget)    { (void)widget; }
    virtual void renderRibbon(Widget& widget)       { (void)widget; }
    virtual void renderTimeline(Widget& widget)     { (void)widget; }
    virtual void renderUnknownWidget(Widget& widget) { (void)widget; }

    virtual void renderCheckBox(Widget& widget)     { (void)widget; }
    virtual void renderTextField(Widget& widget)    { (void)widget; }
    virtual void renderComboBox(Widget& widget)     { (void)widget; }
    virtual void renderSlider(Widget& widget)       { (void)widget; }

    virtual void beginContainer(Widget& ) {}
    virtual void endContainer(Widget& ) {}

private:
    void renderWidgetRecursive(Widget* widget) {
        if (!widget || !widget->isVisible()) return;

        switch (widget->type()) {
            case WidgetType::Button:       renderButton(*widget); break;
            case WidgetType::Label:        renderLabel(*widget); break;
            case WidgetType::Toolbar:      renderToolbar(*widget); break;
            case WidgetType::Tree:         renderTree(*widget); break;
            case WidgetType::PropertyGrid: renderPropertyGrid(*widget); break;
            case WidgetType::Viewport:     renderViewport(*widget); break;
            case WidgetType::Ribbon:       renderRibbon(*widget); break;
            case WidgetType::Timeline:     renderTimeline(*widget); break;
            case WidgetType::CheckBox:     renderCheckBox(*widget); break;
            case WidgetType::TextField:    renderTextField(*widget); break;
            case WidgetType::ComboBox:     renderComboBox(*widget); break;
            case WidgetType::Slider:       renderSlider(*widget); break;
            case WidgetType::Panel:
            case WidgetType::DockPanel:
            case WidgetType::Window:
                renderContainer(*widget);
                break;
            default:
                renderUnknownWidget(*widget);
                break;
        }

        if (!widget->children().empty()) {
            beginContainer(*widget);
            for (Widget* child : widget->children()) {
                renderWidgetRecursive(child);
            }
            endContainer(*widget);
        }
    }
};

}