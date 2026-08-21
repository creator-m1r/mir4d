
#pragma once

#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Rect.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../Document/UIDocument.hpp"

namespace MirUI {

class DragController {
public:

    enum class Operation {
        None,
        Move,
        Resize,
        SelectRect
    };

    explicit DragController(UIDocument& doc);

    void startDrag(Widget* targetWidget,
                   const Point& position,
                   DesignerCanvas::HitZone hitZone);

    void updateDrag(const Point& newPosition);

    bool endDrag();

    void cancelDrag();

    bool isDragging() const;
    Operation currentOperation() const;

private:
    UIDocument& m_doc;
    Operation m_operation = Operation::None;

    WidgetID  m_targetId;
    Point     m_startPos;
    Rect      m_originalBounds;
    Point     m_originalParentPos;

    Rect      m_selectionRect;

    void commitMove();

    void commitResize(const Rect& newBounds);
};

}