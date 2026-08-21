
#pragma once

#include "Event.hpp"
#include "../Widget/Widget.hpp"
#include "../Widget/WidgetTree.hpp"
#include <unordered_map>
#include <vector>
#include <functional>

namespace MirUI {

class EventDispatcher {
public:
    using EventHandler = std::function<void(Event&)>;

    bool dispatch(WidgetTree& tree, Event& event) {
        Widget* target = tree.find(event.target);
        if (!target) return false;

        if (dispatchToWidget(*target, event)) return true;

        return bubble(*target, event);
    }

    bool dispatchToWidget(Widget& widget, Event& event) {
        auto it = m_handlers.find(widget.id());
        if (it != m_handlers.end()) {
            for (auto& handler : it->second) {
                handler(event);
                if (event.handled) return true;
            }
        }
        return event.handled;
    }

    bool bubble(Widget& widget, Event& event) {
        Widget* current = widget.parent();
        while (current) {
            if (dispatchToWidget(*current, event)) return true;
            current = current->parent();
        }
        return false;
    }

    void registerHandler(WidgetID id, EventHandler handler) {
        m_handlers[id].push_back(std::move(handler));
    }

    void unregisterHandlers(WidgetID id) {
        m_handlers.erase(id);
    }

private:
    std::unordered_map<WidgetID, std::vector<EventHandler>> m_handlers;
};

}