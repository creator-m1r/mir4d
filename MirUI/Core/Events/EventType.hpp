// MirUI/Core/Events/EventType.hpp
// Enumeration of all event types recognized by MirUI core.
// Pure C++23, no platform dependencies.

#pragma once

namespace MirUI {

enum class EventType {
    MouseMove,
    MouseDown,
    MouseUp,
    MouseWheel,
    KeyDown,
    KeyUp,
    FocusGained,
    FocusLost,
    Click,
    DoubleClick,
    DragBegin,
    DragMove,
    DragEnd,
    Resize,
    LayoutChanged
};

} // namespace MirUI