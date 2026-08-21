
#include "UIRuntime.hpp"
#include "../Widget/Widget.hpp"
#include <iostream>

namespace MirUI {

bool UIRuntime::initialize() {
    if (m_initialized) return true;
    m_initialized = true;
    return true;
}

void UIRuntime::shutdown() {
    m_initialized = false;
}

void UIRuntime::update(double) {

}

void UIRuntime::render(WidgetTree& tree) {
    if (!m_initialized) return;
    if (Widget* root = tree.root()) {
        renderWidget(root);
    }
}

void UIRuntime::renderWidget(Widget* widget) {
    if (!widget || !widget->isVisible()) return;

    for (Widget* child : widget->children()) {
        renderWidget(child);
    }
}

}