// MirUI/Designer/Canvas/Handles/ResizeHandle.hpp
// 🔲 Ручка изменения размера — маленький квадратик на рамке выделения.
//
// Когда виджет выделен, вокруг него появляется рамка с восемью ручками:
// четыре в углах и четыре посередине сторон. Потянув за такую ручку мышкой,
// пользователь меняет размер виджета. Каждая ручка «знает», где она находится
// (в каком углу или на какой стороне) и какую зону (HitZone) она представляет.
//
// ResizeHandle — это простая структура, которая хранит:
//   • position — координаты центра ручки (в документных координатах)
//   • type     — тип ручки (угол или сторона)
//   • hitZone  — соответствующая зона из HitTest (ResizeLeft, ResizeTopRight и т.д.)
//
// Сами ручки создаются и управляются классом SelectionFrame.
// ResizeHandle только описывает одну конкретную ручку.
// Отрисовкой (маленький квадратик или кружок) занимается Renderer.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../../Core/Layout/Point.hpp"
#include "../HitTest.hpp"  // HitZone

namespace MirUI {

class ResizeHandle {
public:
    // Тип ручки: где именно она находится на рамке.
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

    // ── Конструктор ──────────────────────────────────────────
    // Принимает позицию центра ручки и её тип.
    // HitZone вычисляется автоматически.
    ResizeHandle(const Point& position, Type type)
        : m_position(position)
        , m_type(type)
        , m_hitZone(typeToHitZone(type))
    {}

    // ── Позиция ──────────────────────────────────────────────
    [[nodiscard]] const Point& position() const { return m_position; }

    // ── Тип ручки ────────────────────────────────────────────
    [[nodiscard]] Type type() const { return m_type; }

    // ── Зона hit-теста ───────────────────────────────────────
    [[nodiscard]] HitZone hitZone() const { return m_hitZone; }

private:
    Point m_position;
    Type  m_type;
    HitZone m_hitZone;

    // Преобразует тип ручки в соответствующую HitZone.
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
        return HitZone::None; // на всякий случай
    }
};

} // namespace MirUI