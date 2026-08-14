// MirUI/Designer/Canvas/SnapManager.hpp
// 📐 Менеджер привязки к сетке для холста редактора.
//
// Когда ты перетаскиваешь кнопку мышкой по холсту, она может двигаться
// плавно (на любой пиксель), а может — прыгать только по линиям
// невидимой сетки, например с шагом 8 пикселей. Это называется «привязка»
// или snap. SnapManager умеет превращать произвольные координаты в «притянутые»
// к ближайшему узлу сетки, а также подсказывает, как должны измениться
// размеры виджета, чтобы его края тоже попали на линии сетки.
//
// SnapManager не хранит настройки сетки (размер клетки, включена ли привязка) —
// он получает их извне (от DesignerState или от CanvasModel) и только производит
// вычисления. Это позволяет использовать его и в других местах, где нужна
// привязка к сетке, не привязываясь к конкретному источнику настроек.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Size.hpp"
#include "../../Core/Layout/Rect.hpp"
#include <cmath>

namespace MirUI {

class SnapManager {
public:
    // ── Привязка точки к сетке ───────────────────────────────
    // Принимает точку (например, позицию мыши в координатах документа),
    // размер клетки сетки (gridSize) и флаг enabled.
    // Если enabled == false или gridSize <= 0, возвращает исходную точку.
    // Иначе — координаты округляются до ближайшего числа, кратного gridSize.
    // Пример: gridSize=10, точка (23, 47) → (20, 50).
    [[nodiscard]] static Point snapPosition(const Point& position, double gridSize, bool enabled) {
        if (!enabled || gridSize <= 0.0) return position;
        return Point{
            std::round(position.x / gridSize) * gridSize,
            std::round(position.y / gridSize) * gridSize
        };
    }

    // ── Привязка размера к сетке ─────────────────────────────
    // Принимает размер (width, height) и gridSize.
    // Округляет ширину и высоту до ближайших кратных gridSize.
    // Минимальный размер после привязки — одна клетка.
    [[nodiscard]] static Size snapSize(const Size& size, double gridSize, bool enabled) {
        if (!enabled || gridSize <= 0.0) return size;
        double w = std::round(size.width / gridSize) * gridSize;
        double h = std::round(size.height / gridSize) * gridSize;
        if (w < gridSize) w = gridSize;
        if (h < gridSize) h = gridSize;
        return Size{ w, h };
    }

    // ── Привязка прямоугольника к сетке ──────────────────────
    // Принимает прямоугольник (bounds виджета) и привязывает его левый верхний угол
    // и правый нижний угол к сетке, чтобы все четыре угла лежали на линиях сетки.
    // Возвращает новый прямоугольник.
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

    // ── Привязка смещения (дельта) ───────────────────────────
    // При перетаскивании мы знаем начальную точку и конечную.
    // Мы привязываем обе точки к сетке и возвращаем разницу.
    // Это гарантирует, что при перетаскивании виджет перемещается строго по узлам сетки.
    [[nodiscard]] static Point snapDelta(const Point& from, const Point& to, double gridSize, bool enabled) {
        if (!enabled || gridSize <= 0.0) return Point{ to.x - from.x, to.y - from.y };
        Point snappedFrom = snapPosition(from, gridSize, true);
        Point snappedTo = snapPosition(to, gridSize, true);
        return Point{ snappedTo.x - snappedFrom.x, snappedTo.y - snappedFrom.y };
    }
};

} // namespace MirUI