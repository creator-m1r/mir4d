
#pragma once

#include "Widget.hpp"
#include <unordered_map>
#include <memory>
#include <functional>

namespace MirUI {

class WidgetTree {
public:
    WidgetTree() = default;
    ~WidgetTree() = default;

    void setRoot(std::unique_ptr<Widget> root) {

        m_index.clear();

        m_root = std::move(root);
        if (m_root) {
            registerSubtree(m_root.get());
        }
    }

    [[nodiscard]] Widget* root() const noexcept {
        return m_root.get();
    }

    void registerWidget(Widget* widget) {
        if (widget) {
            m_index[widget->id()] = widget;
        }
    }

    void unregisterWidget(WidgetID id) {
        m_index.erase(id);
    }

    [[nodiscard]] Widget* find(WidgetID id) const {
        auto it = m_index.find(id);
        return (it != m_index.end()) ? it->second : nullptr;
    }

    [[nodiscard]] Widget* findParent(WidgetID id) const {
        Widget* w = find(id);
        return w ? w->parent() : nullptr;
    }

    [[nodiscard]] const std::vector<Widget*>& findChildren(WidgetID id) const {
        Widget* w = find(id);
        if (w) {
            return w->children();
        }
        static const std::vector<Widget*> empty;
        return empty;
    }

    void forEach(const std::function<void(Widget*)>& func) const {
        if (m_root) {
            forEachRecursive(m_root.get(), func);
        }
    }

private:
    std::unique_ptr<Widget> m_root;
    std::unordered_map<WidgetID, Widget*> m_index;

    void registerSubtree(Widget* w) {
        if (!w) return;
        registerWidget(w);
        for (Widget* child : w->children()) {
            registerSubtree(child);
        }
    }

    void forEachRecursive(Widget* w, const std::function<void(Widget*)>& func) const {
        if (!w) return;
        func(w);
        for (Widget* child : w->children()) {
            forEachRecursive(child, func);
        }
    }
};

}