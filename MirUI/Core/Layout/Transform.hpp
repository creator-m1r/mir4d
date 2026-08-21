
#pragma once

#include "Point.hpp"
#include "Size.hpp"

namespace MirUI {

struct Transform {
    Point  position   = Point::zero();
    Size   size       = Size::zero();
    double rotation   = 0.0;
    double scaleX     = 1.0;
    double scaleY     = 1.0;

    Transform() = default;

    Transform(const Point& pos, const Size& sz)
        : position(pos), size(sz) {}

    Transform(const Point& pos, const Size& sz, double rot, double sx = 1.0, double sy = 1.0)
        : position(pos), size(sz), rotation(rot), scaleX(sx), scaleY(sy) {}

    static Transform identity() {
        return {};
    }

    bool operator==(const Transform& other) const = default;
    bool operator!=(const Transform& other) const = default;
};

}