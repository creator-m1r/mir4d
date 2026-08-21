
#pragma once

#include "../../Core/Layout/Rect.hpp"
#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/LayoutData.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../Document/UIDocument.hpp"
#include "GridManager.hpp"
#include "GuideManager.hpp"

namespace MirUI {

class ResizeController {
public:

    enum class Direction {
        None,
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    ResizeController(UIDocument& doc, GridManager& grid, GuideManager& guides);

    void startResize(WidgetID widgetId, Direction direction, const Point& startPos);

    Rect updateResize(const Point& newPos);

    void endResize();

    void cancelResize();

    [[nodiscard]] bool isResizing() const;

private:
    UIDocument& m_doc;
    GridManager& m_grid;
    GuideManager& m_guides;

    bool       m_active = false;
    WidgetID   m_widgetId;
    Direction  m_direction = Direction::None;
    Rect       m_originalBounds;
    Point      m_startPos;
    LayoutData m_layoutData;

    Rect calculateNewBounds(const Point& currentPos) const;

    Rect applyConstraints(const Rect& rect) const;

    Point snapPosition(const Point& pos) const;
};

}