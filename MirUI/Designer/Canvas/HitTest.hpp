// MirUI/Designer/Canvas/HitTest.hpp
// 🎯 Утилиты hit-тестирования для холста редактора.
//
// Когда пользователь наводит мышь на виджет в редакторе, холст должен понять,
// что именно находится под курсором: сам виджет (для перетаскивания),
// его край или угол (для изменения размера) или вообще пустота.
// HitTest содержит простые функции, которые по координатам виджета и позиции
// курсора определяют тип зоны (HitZone).
//
// Зоны:
//   • None           — курсор не над виджетом
//   • Move           — центральная область виджета (можно перетаскивать)
//   • ResizeLeft     — левый край
//   • ResizeRight    — правый край
//   • ResizeTop      — верхний край
//   • ResizeBottom   — нижний край
//   • ResizeTopLeft, TopRight, BottomLeft, BottomRight — углы
//
// Все вычисления производятся в координатах документа (без учёта зума).
// Чувствительность (насколько близко к краю нужно подвести курсор) можно
// настроить через параметр threshold (по умолчанию 6 пикселей).
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Rect.hpp"
#include <algorithm>

namespace MirUI {

// Тип зоны, в которую попал курсор
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
    // ── Основной метод ────────────────────────────────────────
    // Определяет, в какую зону виджета попала точка (позиция курсора).
    //   bounds    — границы виджета в координатах документа
    //   point     — позиция курсора в координатах документа
    //   threshold — расстояние от края, при котором считается, что мы попали в край/угол
    [[nodiscard]] static HitZone detect(const Rect& bounds, const Point& point, double threshold = 6.0) {
        // Сначала проверяем, находится ли точка внутри виджета вообще.
        if (!bounds.contains(point)) {
            return HitZone::None;
        }

        // Вычисляем расстояния до каждой из четырёх сторон.
        double distLeft   = point.x - bounds.x;
        double distRight  = bounds.x + bounds.width - point.x;
        double distTop    = point.y - bounds.y;
        double distBottom = bounds.y + bounds.height - point.y;

        // Флаги: попали ли мы в пороговую зону по каждой из осей.
        bool nearLeft   = (distLeft <= threshold);
        bool nearRight  = (distRight <= threshold);
        bool nearTop    = (distTop <= threshold);
        bool nearBottom = (distBottom <= threshold);

        // Углы имеют приоритет над краями.
        if (nearTop && nearLeft)   return HitZone::ResizeTopLeft;
        if (nearTop && nearRight)  return HitZone::ResizeTopRight;
        if (nearBottom && nearLeft)  return HitZone::ResizeBottomLeft;
        if (nearBottom && nearRight) return HitZone::ResizeBottomRight;

        // Отдельные края.
        if (nearLeft)   return HitZone::ResizeLeft;
        if (nearRight)  return HitZone::ResizeRight;
        if (nearTop)    return HitZone::ResizeTop;
        if (nearBottom) return HitZone::ResizeBottom;

        // Если ни один край не задеты, это центральная зона — перемещение.
        return HitZone::Move;
    }

    // ── Удобный метод для определения, является ли зона угловой ──
    [[nodiscard]] static bool isCorner(HitZone zone) {
        return zone == HitZone::ResizeTopLeft ||
               zone == HitZone::ResizeTopRight ||
               zone == HitZone::ResizeBottomLeft ||
               zone == HitZone::ResizeBottomRight;
    }

    // ── Удобный метод: является ли зона горизонтальным краем ──
    [[nodiscard]] static bool isHorizontalEdge(HitZone zone) {
        return zone == HitZone::ResizeLeft || zone == HitZone::ResizeRight;
    }

    // ── Удобный метод: является ли зона вертикальным краем ──
    [[nodiscard]] static bool isVerticalEdge(HitZone zone) {
        return zone == HitZone::ResizeTop || zone == HitZone::ResizeBottom;
    }
};

} // namespace MirUI