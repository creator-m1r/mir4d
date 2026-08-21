
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

    explicit CanvasModel(UIDocument& document)
        : m_doc(document)
        , m_zoom(1.0)
        , m_offsetX(0.0)
        , m_offsetY(0.0)
    {}

    [[nodiscard]] UIDocument& document() { return m_doc; }
    [[nodiscard]] const UIDocument& document() const { return m_doc; }

    [[nodiscard]] double zoom() const { return m_zoom; }
    void setZoom(double zoom) {
        if (zoom < 0.1) zoom = 0.1;
        if (zoom > 10.0) zoom = 10.0;
        m_zoom = zoom;
    }

    [[nodiscard]] double offsetX() const { return m_offsetX; }
    [[nodiscard]] double offsetY() const { return m_offsetY; }
    void setOffset(double x, double y) {
        m_offsetX = x;
        m_offsetY = y;
    }

    [[nodiscard]] Point screenToDocument(const Point& screenPos) const {
        return Point{
            (screenPos.x - m_offsetX) / m_zoom,
            (screenPos.y - m_offsetY) / m_zoom
        };
    }

    [[nodiscard]] Point documentToScreen(const Point& docPos) const {
        return Point{
            docPos.x * m_zoom + m_offsetX,
            docPos.y * m_zoom + m_offsetY
        };
    }

    [[nodiscard]] WidgetID hitTest(const Point& documentPoint) const {
        Widget* root = m_doc.widgetTree().root();
        if (!root) return WidgetID{};

        Widget* hit = hitTestRecursive(root, documentPoint);
        return hit ? hit->id() : WidgetID{};
    }

    [[nodiscard]] std::vector<WidgetID> widgetsInRect(const Rect& documentRect) const {
        std::vector<WidgetID> result;
        Widget* root = m_doc.widgetTree().root();
        if (root) {
            collectWidgetsInRect(root, documentRect, result);
        }
        return result;
    }

    [[nodiscard]] Rect contentBounds() const {
        Widget* root = m_doc.widgetTree().root();
        if (!root || root->children().empty()) return Rect::zero();

        bool first = true;
        Rect unionRect;
        forEachVisibleWidget(root, [&](Widget* w) {
            if (w == root) return;
            if (first) {
                unionRect = w->bounds();
                first = false;
            } else {
                unionRect = unionRect.unitedWith(w->bounds());
            }
        });
        return unionRect;
    }

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
        if (newZoom > 5.0) newZoom = 1.0;
        setZoom(newZoom);

        double newOffsetX = (viewWidth - bounds.width * m_zoom) * 0.5 - bounds.x * m_zoom;
        double newOffsetY = (viewHeight - bounds.height * m_zoom) * 0.5 - bounds.y * m_zoom;
        setOffset(newOffsetX, newOffsetY);
    }

private:
    UIDocument& m_doc;
    double m_zoom;
    double m_offsetX;
    double m_offsetY;

    static Widget* hitTestRecursive(Widget* widget, const Point& point) {
        if (!widget || !widget->isVisible()) return nullptr;

        const auto& children = widget->children();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            Widget* hit = hitTestRecursive(*it, point);
            if (hit) return hit;
        }

        if (widget->bounds().contains(point)) {
            return widget;
        }
        return nullptr;
    }

    static void collectWidgetsInRect(Widget* widget, const Rect& rect, std::vector<WidgetID>& out) {
        if (!widget || !widget->isVisible()) return;
        if (widget->bounds().intersects(rect)) {

            if (widget->type() != WidgetType::Window && widget->type() != WidgetType::Panel &&
                widget->type() != WidgetType::DockPanel && widget->type() != WidgetType::Toolbar) {
                out.push_back(widget->id());
            }

            for (Widget* child : widget->children()) {
                collectWidgetsInRect(child, rect, out);
            }
        }
    }

    static void forEachVisibleWidget(Widget* widget, const std::function<void(Widget*)>& func) {
        if (!widget || !widget->isVisible()) return;
        func(widget);
        for (Widget* child : widget->children()) {
            forEachVisibleWidget(child, func);
        }
    }
};

}