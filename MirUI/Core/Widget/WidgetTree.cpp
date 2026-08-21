
#include "WidgetTree.hpp"
#include <stdexcept>

namespace MirUI {

void WidgetTree::setRoot(std::unique_ptr<Widget> root) {

    m_index.clear();

    m_root = std::move(root);

    if (m_root) {
        registerSubtree(m_root.get());
    }
}

Widget* WidgetTree::root() const noexcept {
    return m_root.get();
}

void WidgetTree::registerWidget(Widget* widget) {
    if (widget) {

        m_index[widget->id()] = widget;
    }
}

void WidgetTree::unregisterWidget(WidgetID id) {

    m_index.erase(id);
}

Widget* WidgetTree::find(WidgetID id) const {
    auto it = m_index.find(id);
    return (it != m_index.end()) ? it->second : nullptr;
}

Widget* WidgetTree::findParent(WidgetID id) const {
    Widget* w = find(id);
    return w ? w->parent() : nullptr;
}

const std::vector<Widget*>& WidgetTree::findChildren(WidgetID id) const {
    Widget* w = find(id);
    if (w) {
        return w->children();
    }

    static const std::vector<Widget*> empty;
    return empty;
}

void WidgetTree::forEach(const std::function<void(Widget*)>& func) const {
    if (m_root) {
        forEachRecursive(m_root.get(), func);
    }
}

void WidgetTree::registerSubtree(Widget* w) {
    if (!w) return;
    registerWidget(w);
    for (Widget* child : w->children()) {
        registerSubtree(child);
    }
}

void WidgetTree::forEachRecursive(Widget* w, const std::function<void(Widget*)>& func) const {
    if (!w) return;
    func(w);
    for (Widget* child : w->children()) {
        forEachRecursive(child, func);
    }
}

}