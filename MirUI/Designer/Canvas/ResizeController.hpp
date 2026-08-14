// MirUI/Designer/Canvas/ResizeController.hpp
// ↔️ Контроллер изменения размеров виджетов в редакторе MirUI Designer.
//
// Когда пользователь тянет за край или угол выделенного виджета,
// ResizeController вычисляет новые границы (Rect) с учётом:
//   • направления изменения (лево, право, верх, низ, углы)
//   • минимальных и максимальных ограничений (LayoutData виджета)
//   • привязки к сетке (GridManager)
//   • притягивания к направляющим (GuideManager)
//
// Все изменения выполняются через команды (ResizeWidgetCommand),
// поэтому они автоматически попадают в историю Undo/Redo.
//
// Контроллер НЕ обрабатывает события мыши напрямую — он получает
// уже готовые координаты от DesignerCanvas и только вычисляет геометрию.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Layout/Rect.hpp"
#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/LayoutData.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../Document/UIDocument.hpp"
#include "GridManager.hpp"
#include "GuideManager.hpp"

namespace MirUI {

class ResizeController {
public:
    // Направления изменения размера (совпадают с HitZone из DesignerCanvas)
    enum class Direction {
        None,
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    // Конструктор принимает зависимости, необходимые для работы.
    ResizeController(UIDocument& doc, GridManager& grid, GuideManager& guides);

    // ── Начало операции изменения размера ────────────────────
    // Вызывается, когда пользователь нажал кнопку мыши над краем/углом виджета.
    //   widgetId — ID изменяемого виджета
    //   direction — за какой край/угол тянут
    //   startPos — координаты мыши в момент нажатия
    void startResize(WidgetID widgetId, Direction direction, const Point& startPos);

    // ── Обновление при движении мыши ─────────────────────────
    // Вызывается при перемещении мыши с зажатой кнопкой.
    //   newPos — текущие координаты мыши
    // Возвращает предварительные новые границы виджета (для отображения во время перетаскивания).
    Rect updateResize(const Point& newPos);

    // ── Завершение изменения размера ─────────────────────────
    // Вызывается при отпускании кнопки мыши.
    // Создаёт команду ResizeWidgetCommand и добавляет в историю.
    void endResize();

    // ── Отмена (Escape) ──────────────────────────────────────
    void cancelResize();

    // Активно ли сейчас изменение размера?
    [[nodiscard]] bool isResizing() const;

private:
    UIDocument& m_doc;
    GridManager& m_grid;
    GuideManager& m_guides;

    // Сохранённое состояние операции
    bool       m_active = false;
    WidgetID   m_widgetId;
    Direction  m_direction = Direction::None;
    Rect       m_originalBounds;   // границы виджета на момент начала операции
    Point      m_startPos;         // позиция мыши в начале
    LayoutData m_layoutData;       // ограничения виджета (min/max)

    // Вычислить новые границы на основе текущего смещения мыши.
    Rect calculateNewBounds(const Point& currentPos) const;

    // Применить ограничения min/max к вычисленным границам.
    Rect applyConstraints(const Rect& rect) const;

    // Привязка к сетке и направляющим (если включены).
    Point snapPosition(const Point& pos) const;
};

} // namespace MirUI