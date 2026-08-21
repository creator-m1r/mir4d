
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Events/Event.hpp"
#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Rect.hpp"
#include <memory>

namespace MirUI {

class DesignerCanvas {
public:

    explicit DesignerCanvas(UIDocument& document);

    void onMouseDown(const Point& position);
    void onMouseMove(const Point& position);
    void onMouseUp(const Point& position);

    enum class Mode {
        Select,
        AddWidget,
        Pan
    };

    void setMode(Mode mode);
    Mode mode() const;

    void selectInRect(const Rect& rect);

    void selectWidget(WidgetID id);

    void clearSelection();

    void setGridVisible(bool visible);
    bool isGridVisible() const;
    void setSnapToGrid(bool snap);
    bool isSnapToGrid() const;

private:
    UIDocument& m_document;

    enum class DragState {
        None,
        Moving,
        Resizing,
        Selecting
    };

    DragState m_dragState = DragState::None;
    Point m_dragStartPos;
    WidgetID m_dragTarget;
    Rect m_dragOriginalBounds;

    Mode m_mode = Mode::Select;

    bool m_gridVisible = false;
    bool m_snapToGrid = false;

    Widget* hitTest(const Point& position) const;

    enum class HitZone { None, Move, ResizeLeft, ResizeRight, ResizeTop, ResizeBottom,
                         ResizeTopLeft, ResizeTopRight, ResizeBottomLeft, ResizeBottomRight };
    HitZone detectHitZone(Widget* widget, const Point& position) const;

    void commitResize(WidgetID widgetId, const Rect& newBounds);

    void commitMove(WidgetID widgetId, const Point& delta);
};

}