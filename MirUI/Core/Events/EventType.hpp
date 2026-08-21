
#pragma once

namespace MirUI
{

enum class EventType
{
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
    LayoutChanged,
    SelectionChanged,
    SelectionCleared,
    SelectionHoverChanged,
    ObjectCreated,
    ObjectDeleted,
    ObjectModified,
    PropertyChanged,
    DocumentChanged,
    CommandExecuted,
    Undo,
    Redo,
    AICommand,
    ViewportChanged
};

}
