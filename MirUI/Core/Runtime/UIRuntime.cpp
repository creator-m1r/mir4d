// MirUI/Core/Runtime/UIRuntime.cpp
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
    // анимации, hover, таймеры
}

void UIRuntime::render(WidgetTree& tree) {
    if (!m_initialized) return;
    if (Widget* root = tree.root()) {
        renderWidget(root);
    }
}

void UIRuntime::renderWidget(Widget* widget) {
    if (!widget || !widget->isVisible()) return;

    // Здесь будет реальный бэкенд (OpenGL / платформенный рендерер)
    // Пока — обход дерева. Viewport будет рисоваться отдельно через MirEngine OpenGL.

    for (Widget* child : widget->children()) {
        renderWidget(child);
    }
}

} // namespace MirUI