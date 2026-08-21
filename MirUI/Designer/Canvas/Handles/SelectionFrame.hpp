// MirUI/Designer/Canvas/Handles/SelectionFrame.hpp
// 🟦 Рамка выделения — визуальный контур вокруг выбранного виджета.
//
// Когда ты щёлкаешь по кнопке на холсте, вокруг неё появляется синяя рамка
// с маленькими квадратиками по углам и серединам сторон. Эти квадратики —
// «ручки» (handles), потянув за которые можно изменить размер кнопки.
// Рамка и ручки не изменяют сам виджет, а только показывают его границы
// и позволяют взаимодействовать с ними.
//
// SelectionFrame хранит:
//   • bounds       — прямоугольник выделенного виджета (в координатах документа)
//   • handleSize   — размер ручек в пикселях экрана (они не зависят от зума)
//   • ручки (handles) — предвычисленные позиции восьми ручек (4 угла + 4 середины сторон)
//
// Сама рамка ничего не рисует! Она только вычисляет геометрию.
// Отрисовкой занимается платформенный рендерер через RenderCommandBuffer.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../../Core/Layout/Rect.hpp"
#include "../../../Core/Layout/Point.hpp"
#include <vector>

namespace MirUI {

class SelectionFrame {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Создаёт рамку вокруг заданного прямоугольника.
    // handleSize — размер стороны квадратной ручки в пикселях (по умолчанию 8).
    explicit SelectionFrame(const Rect& widgetBounds, double handleSize = 8.0)
        : m_bounds(widgetBounds)
        , m_handleSize(handleSize)
    {
        recalculateHandles();
    }

    // ── Обновление границ ────────────────────────────────────
    // Вызывается, когда виджет изменил размер или переместился.
    void setBounds(const Rect& newBounds) {
        m_bounds = newBounds;
        recalculateHandles();
    }

    [[nodiscard]] const Rect& bounds() const { return m_bounds; }

    // ── Размер ручек ─────────────────────────────────────────
    [[nodiscard]] double handleSize() const { return m_handleSize; }
    void setHandleSize(double size) {
        m_handleSize = size;
        recalculateHandles();
    }

    // ── Доступ к позициям ручек ──────────────────────────────
    // Возвращает список из 8 точек: [0..3] — углы (TL, TR, BR, BL),
    // [4..7] — середины сторон (Top, Right, Bottom, Left).
    [[nodiscard]] const std::vector<Point>& handles() const { return m_handles; }

    // Индексы для удобства
    enum HandleIndex {
        TopLeft = 0,
        TopRight,
        BottomRight,
        BottomLeft,
        MidTop,
        MidRight,
        MidBottom,
        MidLeft
    };

    // ── Hit-тест: попал ли курсор в какую-либо ручку? ───────
    // Принимает позицию курсора в координатах документа.
    // Возвращает индекс ручки (HandleIndex) или -1, если не попал.
    [[nodiscard]] int hitTestHandle(const Point& documentPoint) const {
        double halfSize = m_handleSize * 0.5;
        for (size_t i = 0; i < m_handles.size(); ++i) {
            const Point& h = m_handles[i];
            if (documentPoint.x >= h.x - halfSize &&
                documentPoint.x <= h.x + halfSize &&
                documentPoint.y >= h.y - halfSize &&
                documentPoint.y <= h.y + halfSize) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    // ── Попал ли курсор в саму рамку (не в ручки)? ──────────
    [[nodiscard]] bool hitTestFrame(const Point& documentPoint) const {
        return m_bounds.contains(documentPoint);
    }

private:
    Rect m_bounds;
    double m_handleSize;
    std::vector<Point> m_handles; // 8 точек: углы + середины сторон

    // Пересчитывает позиции всех восьми ручек на основе m_bounds.
    void recalculateHandles() {
        m_handles.clear();
        m_handles.reserve(8);

        double x = m_bounds.x;
        double y = m_bounds.y;
        double w = m_bounds.width;
        double h = m_bounds.height;
        double cx = x + w * 0.5;
        double cy = y + h * 0.5;

        // Углы
        m_handles.emplace_back(x, y);           // TopLeft
        m_handles.emplace_back(x + w, y);       // TopRight
        m_handles.emplace_back(x + w, y + h);   // BottomRight
        m_handles.emplace_back(x, y + h);       // BottomLeft

        // Середины сторон
        m_handles.emplace_back(cx, y);          // MidTop
        m_handles.emplace_back(x + w, cy);      // MidRight
        m_handles.emplace_back(cx, y + h);      // MidBottom
        m_handles.emplace_back(x, cy);          // MidLeft
    }
};

} // namespace MirUI