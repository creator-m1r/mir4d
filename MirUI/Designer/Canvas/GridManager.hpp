// MirUI/Designer/Canvas/GridManager.hpp
// 📐 Менеджер координатной сетки для редактора MirUI Designer.
//
// Когда ты двигаешь кнопку мышкой по холсту, она может прыгать
// не на каждый пиксель, а только по линиям невидимой сетки.
// Это называется «привязка к сетке» (snap-to-grid) и помогает
// аккуратно выравнивать элементы интерфейса.
//
// GridManager хранит настройки этой сетки:
//   - размер одной клетки (например, 8 пикселей)
//   - включена ли привязка
//   - видна ли сетка (показывать линии или нет)
//
// И умеет превращать произвольные координаты мыши в «притянутые»
// к ближайшему узлу сетки — для этого есть метод snap(Point).
// Также умеет корректировать прямоугольники, чтобы их края
// тоже прилипали к линиям (метод snap(Rect)).
//
// Этот класс НЕ рисует сетку сам — он только вычисляет позиции.
// Рисованием занимается Renderer, когда холст просит его отобразить
// направляющие линии.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Rect.hpp"
#include <cmath>

namespace MirUI {

class GridManager {
public:
    // ── Конструктор ──────────────────────────────────────────
    GridManager()
        : m_cellSize(8.0)        // размер клетки по умолчанию — 8 пикселей
        , m_snapEnabled(false)   // привязка выключена, чтобы не мешать
        , m_visible(false)       // сетка не видна
    {}

    // ── Настройка размера клетки ─────────────────────────────
    void setCellSize(double size) {
        // Не даём установить слишком маленькую клетку — минимум 1 пиксель.
        if (size < 1.0) size = 1.0;
        m_cellSize = size;
    }
    [[nodiscard]] double cellSize() const { return m_cellSize; }

    // ── Включение / выключение привязки ──────────────────────
    void setSnapEnabled(bool enabled) { m_snapEnabled = enabled; }
    [[nodiscard]] bool isSnapEnabled() const { return m_snapEnabled; }

    // ── Видимость сетки ──────────────────────────────────────
    void setVisible(bool visible) { m_visible = visible; }
    [[nodiscard]] bool isVisible() const { return m_visible; }

    // ── Привязка точки к сетке ───────────────────────────────
    // Принимает любую точку (например, позицию мыши) и возвращает
    // ближайший узел сетки. Узел — это точка, у которой координаты
    // кратны размеру клетки.
    //
    // Пример:
    //   размер клетки = 10
    //   точка (23, 47) → привяжется к (20, 50)
    //
    // Если привязка выключена, возвращает точку без изменений.
    [[nodiscard]] Point snap(const Point& point) const {
        if (!m_snapEnabled) return point;
        return Point{
            std::round(point.x / m_cellSize) * m_cellSize,
            std::round(point.y / m_cellSize) * m_cellSize
        };
    }

    // ── Привязка прямоугольника к сетке ──────────────────────
    // Принимает прямоугольник (например, границы виджета) и
    // возвращает такой прямоугольник, у которого координаты
    // и размеры привязаны к линиям сетки.
    //
    // Алгоритм: привязываем левый верхний угол, а затем
    // правый нижний, и из них вычисляем новый размер.
    // Это гарантирует, что все четыре угла лежат на узлах сетки.
    //
    // Если привязка выключена, возвращает исходный прямоугольник.
    [[nodiscard]] Rect snap(const Rect& rect) const {
        if (!m_snapEnabled) return rect;

        // Привязываем левый верхний угол
        Point snappedTopLeft = snap(rect.topLeft());

        // Привязываем правый нижний угол
        Point snappedBottomRight = snap(rect.bottomRight());

        // Собираем новый прямоугольник из привязанных углов
        return Rect{
            snappedTopLeft.x,
            snappedTopLeft.y,
            snappedBottomRight.x - snappedTopLeft.x,
            snappedBottomRight.y - snappedTopLeft.y
        };
    }

    // ── Вспомогательный метод: привязка сдвига ───────────────
    // При перетаскивании виджета нам часто нужно привязать
    // не абсолютную позицию, а величину смещения (delta).
    // Этот метод принимает начальную точку и точку назначения,
    // привязывает каждую к сетке и возвращает разницу.
    [[nodiscard]] Point snapDelta(const Point& from, const Point& to) const {
        if (!m_snapEnabled) return {to.x - from.x, to.y - from.y};
        Point snappedFrom = snap(from);
        Point snappedTo   = snap(to);
        return { snappedTo.x - snappedFrom.x, snappedTo.y - snappedFrom.y };
    }

private:
    double m_cellSize;      // размер стороны одной клетки (в пикселях)
    bool   m_snapEnabled;   // включена ли привязка
    bool   m_visible;       // нужно ли рисовать линии сетки
};

} // namespace MirUI