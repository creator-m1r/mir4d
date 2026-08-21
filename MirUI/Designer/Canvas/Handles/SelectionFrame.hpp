
#pragma once

#include "../../../Core/Layout/Rect.hpp"
#include "../../../Core/Layout/Point.hpp"
#include <vector>

namespace MirUI {

class SelectionFrame {
public:

    explicit SelectionFrame(const Rect& widgetBounds, double handleSize = 8.0)
        : m_bounds(widgetBounds)
        , m_handleSize(handleSize)
    {
        recalculateHandles();
    }

    void setBounds(const Rect& newBounds) {
        m_bounds = newBounds;
        recalculateHandles();
    }

    [[nodiscard]] const Rect& bounds() const { return m_bounds; }

    [[nodiscard]] double handleSize() const { return m_handleSize; }
    void setHandleSize(double size) {
        m_handleSize = size;
        recalculateHandles();
    }

    [[nodiscard]] const std::vector<Point>& handles() const { return m_handles; }

    enum HandleIndex {
        TopLeft = 0,
        TopRight,
        BottomRight,
        BottomLeft,
        MidTop,
        MidRight,
        MidBottom,
        MidLeft
    };

    [[nodiscard]] int hitTestHandle(const Point& documentPoint) const {
        double halfSize = m_handleSize * 0.5;
        for (size_t i = 0; i < m_handles.size(); ++i) {
            const Point& h = m_handles[i];
            if (documentPoint.x >= h.x - halfSize &&
                documentPoint.x <= h.x + halfSize &&
                documentPoint.y >= h.y - halfSize &&
                documentPoint.y <= h.y + halfSize) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    [[nodiscard]] bool hitTestFrame(const Point& documentPoint) const {
        return m_bounds.contains(documentPoint);
    }

private:
    Rect m_bounds;
    double m_handleSize;
    std::vector<Point> m_handles;

    void recalculateHandles() {
        m_handles.clear();
        m_handles.reserve(8);

        double x = m_bounds.x;
        double y = m_bounds.y;
        double w = m_bounds.width;
        double h = m_bounds.height;
        double cx = x + w * 0.5;
        double cy = y + h * 0.5;

        m_handles.emplace_back(x, y);
        m_handles.emplace_back(x + w, y);
        m_handles.emplace_back(x + w, y + h);
        m_handles.emplace_back(x, y + h);

        m_handles.emplace_back(cx, y);
        m_handles.emplace_back(x + w, cy);
        m_handles.emplace_back(cx, y + h);
        m_handles.emplace_back(x, cy);
    }
};

}