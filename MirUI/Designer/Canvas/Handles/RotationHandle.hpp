
#pragma once

#include "../../../Core/Layout/Point.hpp"

namespace MirUI {

class RotationHandle {
public:

    RotationHandle(const Point& widgetCenter, double topY, double radius = 30.0)
        : m_center(widgetCenter)
        , m_handlePoint(widgetCenter.x, topY - radius)
        , m_radius(radius)
        , m_visible(true)
    {}

    [[nodiscard]] const Point& handlePoint() const { return m_handlePoint; }

    [[nodiscard]] const Point& center() const { return m_center; }

    [[nodiscard]] double radius() const { return m_radius; }

    [[nodiscard]] bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    void update(const Point& newCenter, double newTopY) {
        m_center = newCenter;
        m_handlePoint = Point(newCenter.x, newTopY - m_radius);
    }

    [[nodiscard]] bool hitTest(const Point& documentPoint, double hitRadius = 8.0) const {
        if (!m_visible) return false;
        double dx = documentPoint.x - m_handlePoint.x;
        double dy = documentPoint.y - m_handlePoint.y;
        return (dx * dx + dy * dy) <= (hitRadius * hitRadius);
    }

private:
    Point  m_center;
    Point  m_handlePoint;
    double m_radius;
    bool   m_visible;
};

}