
#pragma once

#include "../Designer/Document/UIDocument.hpp"
#include "../Core/Events/EventDispatcher.hpp"
#include <string>
#include <memory>

namespace MirUI {

class PlatformAdapter {
public:
    virtual ~PlatformAdapter() = default;

    virtual uint64_t createWindow(const std::string& title, double width, double height) {
        (void)title; (void)width; (void)height;
        return 0;
    }

    virtual void closeWindow(uint64_t windowId) {
        (void)windowId;
    }

    virtual void attachDocument(uint64_t windowId, UIDocument& document) {
        (void)windowId; (void)document;
    }

    virtual void detachDocument(uint64_t windowId) {
        (void)windowId;
    }

    virtual void run() {

    }

    virtual void requestUpdate(uint64_t windowId) {
        (void)windowId;
    }

    [[nodiscard]] EventDispatcher& eventDispatcher() { return m_eventDispatcher; }
    [[nodiscard]] const EventDispatcher& eventDispatcher() const { return m_eventDispatcher; }

protected:
    EventDispatcher m_eventDispatcher;
};

}