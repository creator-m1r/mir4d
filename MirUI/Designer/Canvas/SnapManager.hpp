
#pragma once

#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Size.hpp"
#include "../../Core/Layout/Rect.hpp"
#include <cmath>

namespace MirUI {

class SnapManager {
public:

    [[nodiscard]] static Point snapPosition(const Point& position, double gridSize, bool enabled) {
        if (!enabled || gridSize <= 0.0) return position;
        return Point{
            std::round(position.x / gridSize) * gridSize,
            std::round(position.y / gridSize) * gridSize
        };
    }

    [[nodiscard]] static Size snapSize(const Size& size, double gridSize, bool enabled) {
        if (!enabled || gridSize <= 0.0) return size;
        double w = std::round(size.width / gridSize) * gridSize;
        double h = std::round(size.height / gridSize) * gridSize;
        if (w < gridSize) w = gridSize;
        if (h < gridSize) h = gridSize;
        return Size{ w, h };
    }

    [[nodiscard]] static Rect snapRect(const Rect& rect, double gridSize, bool enabled) {
        if (!enabled || gridSize <= 0.0) return rect;
        Point snappedTopLeft = snapPosition(Point{rect.x, rect.y}, gridSize, true);
        Point snappedBottomRight = snapPosition(
            Point{rect.x + rect.width, rect.y + rect.height}, gridSize, true);
        return Rect{
            snappedTopLeft.x,
            snappedTopLeft.y,
            snappedBottomRight.x - snappedTopLeft.x,
            snappedBottomRight.y - snappedTopLeft.y
        };
    }

    [[nodiscard]] static Point snapDelta(const Point& from, const Point& to, double gridSize, bool enabled) {
        if (!enabled || gridSize <= 0.0) return Point{ to.x - from.x, to.y - from.y };
        Point snappedFrom = snapPosition(from, gridSize, true);
        Point snappedTo = snapPosition(to, gridSize, true);
        return Point{ snappedTo.x - snappedFrom.x, snappedTo.y - snappedFrom.y };
    }
};

}