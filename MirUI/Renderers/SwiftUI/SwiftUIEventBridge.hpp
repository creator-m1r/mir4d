
#pragma once

#include "../../Core/Bridge/UIEventBridge.hpp"
#include "../../Core/Events/Event.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <memory>

namespace MirUI {

#ifdef __OBJC__
@class NSEvent;
#else
struct NSEvent;
#endif

class SwiftUIEventBridge : public UIEventBridge {
public:
    SwiftUIEventBridge();
    virtual ~SwiftUIEventBridge();

    bool handleNSEvent(const NSEvent* nsEvent, WidgetID targetWidget);

    Event createEvent() override;

    void dispatchPlatformEvent(const Event& event);

private:

    struct Impl;
    std::unique_ptr<Impl> m_impl;

    static EventType nsEventTypeToMirUI(long nsType);

    static void extractMouseCoordinates(const NSEvent* nsEvent, double& x, double& y);

    static void extractKeyInfo(const NSEvent* nsEvent, int& keyCode, bool& ctrl, bool& shift, bool& alt, bool& cmd);
};

}