// MirUI/Designer/Canvas/DragController.hpp
// 🖱️ Контроллер перетаскивания виджетов в редакторе MirUI Designer.
//
// DragController отвечает за логику, которая включается, когда
// пользователь нажимает кнопку мыши на виджете и, не отпуская,
// перемещает курсор. Это может быть:
//   • Обычное перемещение виджета (drag & drop) — изменение позиции.
//   • Изменение размера (resize) — потянули за уголок или край.
//   • Выделение области (rubber band selection) — растягивание рамки.
//
// DragController НЕ знает о платформенных событиях напрямую.
// Он получает от DesignerCanvas координаты в логических точках
// и командует документу через ICommand (MoveWidgetCommand и ResizeWidgetCommand),
// автоматически поддерживая Undo/Redo.
//
// Пример работы:
//   1. Пользователь нажимает мышь над кнопкой. Холст вызывает startDrag.
//   2. Пользователь тянет мышь. Холст вызывает updateDrag с новыми координатами.
//   3. Пользователь отпускает мышь. Холст вызывает endDrag.
//      Контроллер фиксирует итоговое положение и создаёт команду для CommandHistory.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Rect.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../Document/UIDocument.hpp"

namespace MirUI {

class DragController {
public:
    // Типы операций, которые может выполнять контроллер.
    enum class Operation {
        None,
        Move,          // перетаскивание виджета
        Resize,        // изменение размера за край/угол
        SelectRect     // выделение прямоугольной области
    };

    // Конструктор принимает документ, в котором работает.
    explicit DragController(UIDocument& doc);

    // ── Начало перетаскивания ────────────────────────────────
    // Вызывается при нажатии кнопки мыши.
    //   targetWidget — виджет, над которым нажали (может быть nullptr).
    //   position     — координаты курсора в логических точках.
    //   hitZone      — зона, за которую схватились (из DesignerCanvas::detectHitZone).
    void startDrag(Widget* targetWidget,
                   const Point& position,
                   DesignerCanvas::HitZone hitZone);

    // ── Обновление при движении мыши ─────────────────────────
    // Вызывается при перемещении мыши с зажатой кнопкой.
    //   newPosition — новые координаты курсора.
    void updateDrag(const Point& newPosition);

    // ── Завершение перетаскивания ────────────────────────────
    // Вызывается при отпускании кнопки мыши.
    // Возвращает true, если в результате перетаскивания были реальные изменения.
    bool endDrag();

    // ── Отмена (например, при нажатии Escape) ────────────────
    void cancelDrag();

    // ── Состояние ─────────────────────────────────────────────
    bool isDragging() const;
    Operation currentOperation() const;

private:
    UIDocument& m_doc;               // документ, который редактируем
    Operation m_operation = Operation::None;

    // Исходные данные, запомненные в момент startDrag
    WidgetID  m_targetId;            // ID перетаскиваемого виджета
    Point     m_startPos;            // начальная позиция курсора
    Rect      m_originalBounds;      // исходные границы виджета до операции
    Point     m_originalParentPos;   // позиция родителя (для расчёта новых координат)

    // Для выделения области
    Rect      m_selectionRect;       // текущая рамка выделения

    // Вспомогательные методы
    // Создать и выполнить команду перемещения (MoveWidgetCommand).
    void commitMove();

    // Создать и выполнить команду изменения размера.
    void commitResize(const Rect& newBounds);
};

} // namespace MirUI