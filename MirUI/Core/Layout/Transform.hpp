// MirUI/Core/Layout/Transform.hpp
// 🔄 Структура трансформации виджета — положение, размер, поворот и масштаб.
//
// Виджет в MirUI располагается на экране не просто по координатам x и y.
// У него может быть собственный угол поворота и коэффициент масштабирования
// (например, для анимаций, эффектов или будущих XR-интерфейсов).
// Transform объединяет все эти параметры в одну удобную структуру.
//
// Эта структура используется:
//   • В LayoutData для полного описания геометрии виджета.
//   • В DesignerCanvas для перемещения, вращения и изменения размера мышкой.
//   • В рендерерах (SwiftUI, WinUI) для применения аффинных преобразований.
//   • В системе анимаций для плавного изменения позиции, размера и угла.
//
// Поля:
//   • position   — координаты левого верхнего угла виджета (Point)
//   • size       — размеры виджета (Size)
//   • rotation   — угол поворота в градусах (по часовой стрелке, 0 — без поворота)
//   • scaleX     — масштаб по горизонтали (1.0 — исходный размер)
//   • scaleY     — масштаб по вертикали (1.0 — исходный размер)
//
// Все поля имеют разумные значения по умолчанию: нулевая позиция, нулевой размер,
// без поворота, без масштабирования.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "Point.hpp"
#include "Size.hpp"

namespace MirUI {

struct Transform {
    Point  position   = Point::zero();   // где находится виджет (x, y)
    Size   size       = Size::zero();    // его ширина и высота
    double rotation   = 0.0;             // угол поворота в градусах
    double scaleX     = 1.0;             // масштаб по горизонтали (1.0 = 100%)
    double scaleY     = 1.0;             // масштаб по вертикали

    // ── Конструкторы ──────────────────────────────────────────
    Transform() = default;

    Transform(const Point& pos, const Size& sz)
        : position(pos), size(sz) {}

    Transform(const Point& pos, const Size& sz, double rot, double sx = 1.0, double sy = 1.0)
        : position(pos), size(sz), rotation(rot), scaleX(sx), scaleY(sy) {}

    // ── Удобные методы ───────────────────────────────────────
    static Transform identity() {
        return {};
    }

    bool operator==(const Transform& other) const = default;
    bool operator!=(const Transform& other) const = default;
};

} // namespace MirUI