
#pragma once

#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Rect.hpp"
#include <cmath>

namespace MirUI {

class GridManager {
public:

    GridManager()
        : m_cellSize(8.0)
        , m_snapEnabled(false)
        , m_visible(false)
    {}

    void setCellSize(double size) {

        if (size < 1.0) size = 1.0;
        m_cellSize = size;
    }
    [[nodiscard]] double cellSize() const { return m_cellSize; }

    void setSnapEnabled(bool enabled) { m_snapEnabled = enabled; }
    [[nodiscard]] bool isSnapEnabled() const { return m_snapEnabled; }

    void setVisible(bool visible) { m_visible = visible; }
    [[nodiscard]] bool isVisible() const { return m_visible; }

    [[nodiscard]] Point snap(const Point& point) const {
        if (!m_snapEnabled) return point;
        return Point{
            std::round(point.x / m_cellSize) * m_cellSize,
            std::round(point.y / m_cellSize) * m_cellSize
        };
    }

    [[nodiscard]] Rect snap(const Rect& rect) const {
        if (!m_snapEnabled) return rect;

        Point snappedTopLeft = snap(rect.topLeft());

        Point snappedBottomRight = snap(rect.bottomRight());

        return Rect{
            snappedTopLeft.x,
            snappedTopLeft.y,
            snappedBottomRight.x - snappedTopLeft.x,
            snappedBottomRight.y - snappedTopLeft.y
        };
    }

    [[nodiscard]] Point snapDelta(const Point& from, const Point& to) const {
        if (!m_snapEnabled) return {to.x - from.x, to.y - from.y};
        Point snappedFrom = snap(from);
        Point snappedTo   = snap(to);
        return { snappedTo.x - snappedFrom.x, snappedTo.y - snappedFrom.y };
    }

private:
    double m_cellSize;
    bool   m_snapEnabled;
    bool   m_visible;
};

}