// MirUI/Designer/Canvas/CanvasModel.hpp
// 🖼️ Модель данных холста — сердце визуального редактора.
//
// Холст DesignerCanvas показывает интерфейс, который мы редактируем.
// Но сам холст не хранит виджеты — они хранятся в UIDocument (в WidgetTree).
// CanvasModel — это прослойка, которая связывает визуальное представление холста
// с деревом виджетов. Она предоставляет удобные методы, которые нужны именно
// для редактирования: поиск виджета под курсором, получение списка виджетов
// в прямоугольной области (для рамки выделения), пересчёт координат с учётом
// масштаба (zoom) и смещения (scroll).
//
// Когда мы крутим колёсико мыши или тянем холст, мы не двигаем сами виджеты,
// а меняем масштаб и смещение в CanvasModel. Все координаты, которые приходят
// от мыши, CanvasModel пересчитывает из «экранных» в «документные»,
// чтобы все изменения применялись к правильным координатам виджетов.
//
// CanvasModel НЕ занимается отрисовкой — только математикой и доступом к данным.
// Рисует холст Renderer (SwiftUI/WinUI), а управляет взаимодействием DesignerCanvas.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Layout/Point.hpp"
#include "../../Core/Layout/Rect.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <vector>
#include <functional>

namespace MirUI {

class CanvasModel {
public:
    // ── Конструктор ──────────────────────────────────────────
    explicit CanvasModel(UIDocument& document)
        : m_doc(document)
        , m_zoom(1.0)
        , m_offsetX(0.0)
        , m_offsetY(0.0)
    {}

    // ── Документ ─────────────────────────────────────────────
    [[nodiscard]] UIDocument& document() { return m_doc; }
    [[nodiscard]] const UIDocument& document() const { return m_doc; }

    // ── Масштаб и смещение холста ────────────────────────────

    // Масштаб: 1.0 = 100% (пиксель в пиксель), 2.0 = 200% и т.д.
    [[nodiscard]] double zoom() const { return m_zoom; }
    void setZoom(double zoom) {
        if (zoom < 0.1) zoom = 0.1;
        if (zoom > 10.0) zoom = 10.0;
        m_zoom = zoom;
    }

    // Смещение холста в пикселях экрана (например, при панорамировании).
    [[nodiscard]] double offsetX() const { return m_offsetX; }
    [[nodiscard]] double offsetY() const { return m_offsetY; }
    void setOffset(double x, double y) {
        m_offsetX = x;
        m_offsetY = y;
    }

    // ── Преобразование координат ─────────────────────────────
    // Превращает экранные координаты мыши в координаты документа.
    //   экранные — то, что приходит от платформенного рендерера.
    //   документные — то, что хранится в Rect виджетов.
    [[nodiscard]] Point screenToDocument(const Point& screenPos) const {
        return Point{
            (screenPos.x - m_offsetX) / m_zoom,
            (screenPos.y - m_offsetY) / m_zoom
        };
    }

    // Обратное преобразование: из документа в экран (для отрисовки).
    [[nodiscard]] Point documentToScreen(const Point& docPos) const {
        return Point{
            docPos.x * m_zoom + m_offsetX,
            docPos.y * m_zoom + m_offsetY
        };
    }

    // ── Hit-тестирование: поиск виджета под курсором ─────────
    // Возвращает ID самого глубокого виджета, который содержит точку (в координатах документа).
    // Если ни один виджет не найден, возвращает невалидный ID (0).
    [[nodiscard]] WidgetID hitTest(const Point& documentPoint) const {
        Widget* root = m_doc.widgetTree().root();
        if (!root) return WidgetID{};

        // Ищем рекурсивно, начиная с корня. Обходим детей в обратном порядке,
        // чтобы верхний (последний нарисованный) виджет был найден первым.
        Widget* hit = hitTestRecursive(root, documentPoint);
        return hit ? hit->id() : WidgetID{};
    }

    // ── Поиск виджетов в прямоугольной области ───────────────
    // Возвращает список ID всех видимых виджетов, чьи bounds пересекаются с rect (в документных координатах).
    // Используется для выделения рамкой (rubber band selection).
    [[nodiscard]] std::vector<WidgetID> widgetsInRect(const Rect& documentRect) const {
        std::vector<WidgetID> result;
        Widget* root = m_doc.widgetTree().root();
        if (root) {
            collectWidgetsInRect(root, documentRect, result);
        }
        return result;
    }

    // ── Границы всего холста ─────────────────────────────────
    // Возвращает прямоугольник, охватывающий всех видимых потомков корня (в документных координатах).
    // Если дерево пустое, возвращает Rect::zero().
    [[nodiscard]] Rect contentBounds() const {
        Widget* root = m_doc.widgetTree().root();
        if (!root || root->children().empty()) return Rect::zero();

        // Начинаем с bounds первого ребёнка и расширяем.
        bool first = true;
        Rect unionRect;
        forEachVisibleWidget(root, [&](Widget* w) {
            if (w == root) return; // корень не учитываем
            if (first) {
                unionRect = w->bounds();
                first = false;
            } else {
                unionRect = unionRect.unitedWith(w->bounds());
            }
        });
        return unionRect;
    }

    // ── Подогнать масштаб, чтобы всё содержимое было видно ───
    // Вычисляет zoom и offset так, чтобы contentBounds полностью поместились
    // в заданный размер области просмотра (viewWidth, viewHeight).
    void fitContent(double viewWidth, double viewHeight, double padding = 40.0) {
        Rect bounds = contentBounds();
        if (bounds.width <= 0 || bounds.height <= 0) {
            setZoom(1.0);
            setOffset(0, 0);
            return;
        }

        double availableW = viewWidth - padding * 2;
        double availableH = viewHeight - padding * 2;
        double scaleX = availableW / bounds.width;
        double scaleY = availableH / bounds.height;
        double newZoom = std::min(scaleX, scaleY);
        if (newZoom > 5.0) newZoom = 1.0; // не увеличивать слишком сильно
        setZoom(newZoom);

        // Центрируем.
        double newOffsetX = (viewWidth - bounds.width * m_zoom) * 0.5 - bounds.x * m_zoom;
        double newOffsetY = (viewHeight - bounds.height * m_zoom) * 0.5 - bounds.y * m_zoom;
        setOffset(newOffsetX, newOffsetY);
    }

private:
    UIDocument& m_doc;
    double m_zoom;
    double m_offsetX;
    double m_offsetY;

    // Рекурсивный обход виджетов для hit-теста (с учётом видимости).
    static Widget* hitTestRecursive(Widget* widget, const Point& point) {
        if (!widget || !widget->isVisible()) return nullptr;
        // Проверяем детей в обратном порядке (последний ребёнок — самый верхний).
        const auto& children = widget->children();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            Widget* hit = hitTestRecursive(*it, point);
            if (hit) return hit;
        }
        // Если ни один ребёнок не подошёл, проверяем сам виджет.
        if (widget->bounds().contains(point)) {
            return widget;
        }
        return nullptr;
    }

    // Рекурсивный сбор виджетов, попавших в прямоугольник.
    static void collectWidgetsInRect(Widget* widget, const Rect& rect, std::vector<WidgetID>& out) {
        if (!widget || !widget->isVisible()) return;
        if (widget->bounds().intersects(rect)) {
            // Если виджет — не контейнер и пересекается, добавляем его.
            // Контейнеры (Panel, Window) обычно не выделяются как самостоятельные элементы.
            if (widget->type() != WidgetType::Window && widget->type() != WidgetType::Panel &&
                widget->type() != WidgetType::DockPanel && widget->type() != WidgetType::Toolbar) {
                out.push_back(widget->id());
            }
            // Обходим детей.
            for (Widget* child : widget->children()) {
                collectWidgetsInRect(child, rect, out);
            }
        }
    }

    // Вспомогательный метод обхода всех видимых виджетов.
    static void forEachVisibleWidget(Widget* widget, const std::function<void(Widget*)>& func) {
        if (!widget || !widget->isVisible()) return;
        func(widget);
        for (Widget* child : widget->children()) {
            forEachVisibleWidget(child, func);
        }
    }
};

} // namespace MirUI