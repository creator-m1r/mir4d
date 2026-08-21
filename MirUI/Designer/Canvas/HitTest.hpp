
#pragma once

#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Rect.hpp"
#include <algorithm>

namespace MirUI {

enum class HitZone {
    None,
    Move,
    ResizeLeft,
    ResizeRight,
    ResizeTop,
    ResizeBottom,
    ResizeTopLeft,
    ResizeTopRight,
    ResizeBottomLeft,
    ResizeBottomRight
};

class HitTest {
public:

    [[nodiscard]] static HitZone detect(const Rect& bounds, const Point& point, double threshold = 6.0) {

        if (!bounds.contains(point)) {
            return HitZone::None;
        }

        double distLeft   = point.x - bounds.x;
        double distRight  = bounds.x + bounds.width - point.x;
        double distTop    = point.y - bounds.y;
        double distBottom = bounds.y + bounds.height - point.y;

        bool nearLeft   = (distLeft <= threshold);
        bool nearRight  = (distRight <= threshold);
        bool nearTop    = (distTop <= threshold);
        bool nearBottom = (distBottom <= threshold);

        if (nearTop && nearLeft)   return HitZone::ResizeTopLeft;
        if (nearTop && nearRight)  return HitZone::ResizeTopRight;
        if (nearBottom && nearLeft)  return HitZone::ResizeBottomLeft;
        if (nearBottom && nearRight) return HitZone::ResizeBottomRight;

        if (nearLeft)   return HitZone::ResizeLeft;
        if (nearRight)  return HitZone::ResizeRight;
        if (nearTop)    return HitZone::ResizeTop;
        if (nearBottom) return HitZone::ResizeBottom;

        return HitZone::Move;
    }

    [[nodiscard]] static bool isCorner(HitZone zone) {
        return zone == HitZone::ResizeTopLeft ||
               zone == HitZone::ResizeTopRight ||
               zone == HitZone::ResizeBottomLeft ||
               zone == HitZone::ResizeBottomRight;
    }

    [[nodiscard]] static bool isHorizontalEdge(HitZone zone) {
        return zone == HitZone::ResizeLeft || zone == HitZone::ResizeRight;
    }

    [[nodiscard]] static bool isVerticalEdge(HitZone zone) {
        return zone == HitZone::ResizeTop || zone == HitZone::ResizeBottom;
    }
};

}