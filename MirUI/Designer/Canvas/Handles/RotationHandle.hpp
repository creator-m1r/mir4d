// MirUI/Designer/Canvas/Handles/RotationHandle.hpp
// 🔄 Ручка поворота — специальная ручка, позволяющая вращать выделенный виджет.
//
// Когда виджет выделен, помимо квадратных ручек изменения размера по углам и сторонам,
// над верхней гранью рамки появляется круглая ручка поворота. Потянув за неё мышкой,
// пользователь может вращать виджет вокруг его центра.
//
// RotationHandle хранит:
//   • center       — центр виджета, вокруг которого происходит вращение (в документных координатах)
//   • handlePoint  — точка на окружности, за которую тянет пользователь (обычно над верхней гранью)
//   • radius       — расстояние от центра до ручки (радиус окружности вращения)
//   • visible      — показывать ли ручку (может быть скрыта, если виджет не поддерживает вращение)
//
// Сама ручка не выполняет вычисление угла — она только предоставляет геометрические данные.
// Вычислением нового угла занимается RotateController или DragController на основе координат мыши.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../../Core/Layout/Point.hpp"

namespace MirUI {

class RotationHandle {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Принимает центр виджета и расстояние от центра до ручки (по умолчанию 30 пикселей над верхней гранью).
    // Положение ручки вычисляется автоматически: она находится над центром верхней грани.
    RotationHandle(const Point& widgetCenter, double topY, double radius = 30.0)
        : m_center(widgetCenter)
        , m_handlePoint(widgetCenter.x, topY - radius) // над центром верхней грани
        , m_radius(radius)
        , m_visible(true)
    {}

    // ── Позиция ручки ────────────────────────────────────────
    [[nodiscard]] const Point& handlePoint() const { return m_handlePoint; }

    // ── Центр вращения ───────────────────────────────────────
    [[nodiscard]] const Point& center() const { return m_center; }

    // ── Радиус окружности ────────────────────────────────────
    [[nodiscard]] double radius() const { return m_radius; }

    // ── Видимость ────────────────────────────────────────────
    [[nodiscard]] bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    // ── Обновление позиции (при изменении размеров/положения виджета) ──
    void update(const Point& newCenter, double newTopY) {
        m_center = newCenter;
        m_handlePoint = Point(newCenter.x, newTopY - m_radius);
    }

    // ── Hit-тест: попал ли курсор в ручку? ──────────────────
    // Принимает позицию курсора в координатах документа и размер "зоны попадания".
    // По умолчанию зона — круг радиусом 8 пикселей вокруг handlePoint.
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

} // namespace MirUI