
#pragma once

#include "../../../Core/Layout/Point.hpp"
#include "../HitTest.hpp"

namespace MirUI {

class ResizeHandle {
public:

    enum class Type {
        TopLeft,
        TopRight,
        BottomRight,
        BottomLeft,
        MidTop,
        MidRight,
        MidBottom,
        MidLeft
    };

    ResizeHandle(const Point& position, Type type)
        : m_position(position)
        , m_type(type)
        , m_hitZone(typeToHitZone(type))
    {}

    [[nodiscard]] const Point& position() const { return m_position; }

    [[nodiscard]] Type type() const { return m_type; }

    [[nodiscard]] HitZone hitZone() const { return m_hitZone; }

private:
    Point m_position;
    Type  m_type;
    HitZone m_hitZone;

    static HitZone typeToHitZone(Type type) {
        switch (type) {
            case Type::TopLeft:     return HitZone::ResizeTopLeft;
            case Type::TopRight:    return HitZone::ResizeTopRight;
            case Type::BottomRight: return HitZone::ResizeBottomRight;
            case Type::BottomLeft:  return HitZone::ResizeBottomLeft;
            case Type::MidTop:      return HitZone::ResizeTop;
            case Type::MidRight:    return HitZone::ResizeRight;
            case Type::MidBottom:   return HitZone::ResizeBottom;
            case Type::MidLeft:     return HitZone::ResizeLeft;
        }
        return HitZone::None;
    }
};

}