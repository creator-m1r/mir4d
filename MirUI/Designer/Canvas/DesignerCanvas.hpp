// MirUI/Designer/Canvas/DesignerCanvas.hpp
// 🎨 Холст визуального редактора MirUI Designer.
//
// Главная задача DesignerCanvas — показать пользователю его интерфейс
// в том виде, в каком он будет выглядеть в реальном приложении,
// и позволить менять расположение, размеры и структуру виджетов
// простым перетаскиванием мыши. Холст НЕ рисует сам — он только
// управляет логикой взаимодействия, а всю отрисовку делегирует
// универсальному MirUI::Renderer (SwiftUI, WinUI, NullRenderer).
//
// Когда ты двигаешь мышкой по холсту:
//   1. Платформенный рендерер ловит события (клик, движение, отпускание).
//   2. События попадают в DesignerCanvas.
//   3. Холст определяет, над каким виджетом сейчас курсор,
//      нужно ли начать перетаскивание или изменение размера.
//   4. Холст создаёт команду (MoveWidgetCommand, ResizeWidgetCommand…)
//      и помещает её в CommandHistory документа.
//   5. Команда изменяет WidgetTree.
//   6. Рендерер перерисовывает изменённый интерфейс.
//
// Таким образом, ты видишь результат мгновенно, а любое действие
// можно отменить через Ctrl+Z.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Events/Event.hpp"
#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Rect.hpp"
#include <memory>

namespace MirUI {

class DesignerCanvas {
public:
    // Конструктор принимает ссылку на документ, который будем редактировать.
    explicit DesignerCanvas(UIDocument& document);

    // ── Обработчики событий от платформенного рендерера ──────
    // Эти методы вызываются, когда пользователь нажимает кнопку мыши,
    // двигает курсор или отпускает кнопку над холстом.

    void onMouseDown(const Point& position);
    void onMouseMove(const Point& position);
    void onMouseUp(const Point& position);

    // ── Режимы работы холста ─────────────────────────────────
    enum class Mode {
        Select,     // обычное выделение и перетаскивание
        AddWidget,  // добавление нового виджета из тулбокса
        Pan         // прокрутка холста (когда интерфейс больше экрана)
    };

    void setMode(Mode mode);
    Mode mode() const;

    // ── Выделение ────────────────────────────────────────────
    // Выделить все виджеты, попавшие в прямоугольную область.
    void selectInRect(const Rect& rect);
    // Выделить один виджет.
    void selectWidget(WidgetID id);
    // Очистить выделение.
    void clearSelection();

    // ── Направляющие и сетка ─────────────────────────────────
    void setGridVisible(bool visible);
    bool isGridVisible() const;
    void setSnapToGrid(bool snap);
    bool isSnapToGrid() const;

private:
    UIDocument& m_document;     // документ, который мы редактируем

    // Состояние перетаскивания
    enum class DragState {
        None,
        Moving,      // перетаскиваем виджет(ы)
        Resizing,    // тянем за уголок — меняем размер
        Selecting    // растягиваем рамку выделения
    };

    DragState m_dragState = DragState::None;
    Point m_dragStartPos;       // точка, где началось перетаскивание
    WidgetID m_dragTarget;      // какой виджет перетаскиваем / изменяем
    Rect m_dragOriginalBounds;  // исходные границы виджета до изменения

    // Режим работы
    Mode m_mode = Mode::Select;

    // Визуальные настройки
    bool m_gridVisible = false;
    bool m_snapToGrid = false;

    // ── Внутренние помощники ─────────────────────────────────
    // Найти виджет под курсором (самый глубокий, который visible).
    Widget* hitTest(const Point& position) const;

    // Проверить, попал ли курсор в зону изменения размера
    // (край или угол виджета) и вернуть тип операции.
    enum class HitZone { None, Move, ResizeLeft, ResizeRight, ResizeTop, ResizeBottom,
                         ResizeTopLeft, ResizeTopRight, ResizeBottomLeft, ResizeBottomRight };
    HitZone detectHitZone(Widget* widget, const Point& position) const;

    // Создать и выполнить команду для изменения размера.
    void commitResize(WidgetID widgetId, const Rect& newBounds);

    // Создать и выполнить команду для перемещения.
    void commitMove(WidgetID widgetId, const Point& delta);
};

} // namespace MirUI