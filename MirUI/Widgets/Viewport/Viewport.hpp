// MirUI/Widgets/Viewport/Viewport.hpp
// Виджет «Вьюпорт» — область, в которой будет отображаться 3D-сцена.
// Сам 3D-движок находится в MirEngine, а здесь только описание того,
// что должно быть показано: сетка, оси, гизмо, а также события мыши.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <string>
#include <functional>

namespace MirUI {

// Тип для идентификатора вьюпорта (пока что просто строка, позже можно заменить на ID).
using ViewportID = std::string;

class Viewport : public Widget {
public:
    // Создаём виджет с типом Viewport.
    Viewport()
        : Widget(WidgetType::Viewport)
    {}

    // ── Идентификатор вьюпорта ────────────────────────────────
    // Каждый вьюпорт может иметь уникальное имя (например, "Perspective", "Top", "Front").
    void setViewportID(const ViewportID& id) { m_viewportID = id; }
    [[nodiscard]] const ViewportID& viewportID() const { return m_viewportID; }

    // ── Визуальные подсказки ─────────────────────────────────

    // Показывать ли координатную сетку.
    void setGridVisible(bool visible) { m_gridVisible = visible; }
    [[nodiscard]] bool isGridVisible() const { return m_gridVisible; }

    // Показывать ли цветные оси координат.
    void setAxesVisible(bool visible) { m_axesVisible = visible; }
    [[nodiscard]] bool isAxesVisible() const { return m_axesVisible; }

    // Показывать ли гизмо (манипулятор перемещения/вращения/масштаба).
    void setGizmoVisible(bool visible) { m_gizmoVisible = visible; }
    [[nodiscard]] bool isGizmoVisible() const { return m_gizmoVisible; }

    // ── Обработчики событий указателя ────────────────────────
    // Эти колбэки будут вызываться из EventDispatcher, когда пользователь
    // нажимает кнопки мыши или двигает курсор над вьюпортом.
    // Позже их заменит полноценная система событий, а пока — прямые колбэки для тестов.

    using PointerCallback = std::function<void(Viewport&, double x, double y)>;

    void setOnPointerDown(PointerCallback callback) { m_onPointerDown = std::move(callback); }
    void setOnPointerMove(PointerCallback callback) { m_onPointerMove = std::move(callback); }
    void setOnPointerUp(PointerCallback callback)   { m_onPointerUp   = std::move(callback); }

    // Эти методы будут вызываться из адаптера рендерера, когда приходит событие от ОС.
    void handlePointerDown(double x, double y) {
        if (m_onPointerDown) m_onPointerDown(*this, x, y);
    }
    void handlePointerMove(double x, double y) {
        if (m_onPointerMove) m_onPointerMove(*this, x, y);
    }
    void handlePointerUp(double x, double y) {
        if (m_onPointerUp) m_onPointerUp(*this, x, y);
    }

private:
    ViewportID m_viewportID;      // Имя вьюпорта, например "Perspective".
    bool m_gridVisible  = true;   // Показывать сетку?
    bool m_axesVisible  = true;   // Показывать оси?
    bool m_gizmoVisible = true;   // Показывать гизмо?

    // Колбэки для событий указателя (будут заменены на CommandBus/EventDispatcher позже).
    PointerCallback m_onPointerDown;
    PointerCallback m_onPointerMove;
    PointerCallback m_onPointerUp;
};

} // namespace MirUI