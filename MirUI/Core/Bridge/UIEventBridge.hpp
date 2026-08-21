
#pragma once

#include "../Events/Event.hpp"
#include "../Widget/WidgetID.hpp"
#include "../Widget/WidgetTree.hpp"
#include "../Events/EventDispatcher.hpp"
#include <functional>
#include <memory>

namespace MirUI {

class UIEventBridge {
public:
    virtual ~UIEventBridge() = default;

    void setEventDispatcher(EventDispatcher* dispatcher) {
        m_dispatcher = dispatcher;
    }

    void setWidgetTree(WidgetTree* tree) {
        m_widgetTree = tree;
    }

    virtual void dispatchEvent(const Event& event) {
        if (m_dispatcher && m_widgetTree) {
            Event mutableEvent = event;
            m_dispatcher->dispatch(*m_widgetTree, mutableEvent);
        }
    }

    virtual Event createEvent() = 0;

    using OutgoingEventCallback = std::function<void(const Event&)>;
    void setOutgoingEventCallback(OutgoingEventCallback callback) {
        m_outgoingCallback = std::move(callback);
    }

protected:
    EventDispatcher* m_dispatcher = nullptr;
    WidgetTree*      m_widgetTree = nullptr;
    OutgoingEventCallback m_outgoingCallback;

    void sendToPlatform(const Event& event) {
        if (m_outgoingCallback) {
            m_outgoingCallback(event);
        }
    }
};

}