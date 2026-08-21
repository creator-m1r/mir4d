
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <string>
#include <functional>

namespace MirUI {

using ViewportID = std::string;

class Viewport : public Widget {
public:

    Viewport()
        : Widget(WidgetType::Viewport)
    {}

    void setViewportID(const ViewportID& id) { m_viewportID = id; }
    [[nodiscard]] const ViewportID& viewportID() const { return m_viewportID; }

    void setGridVisible(bool visible) { m_gridVisible = visible; }
    [[nodiscard]] bool isGridVisible() const { return m_gridVisible; }

    void setAxesVisible(bool visible) { m_axesVisible = visible; }
    [[nodiscard]] bool isAxesVisible() const { return m_axesVisible; }

    void setGizmoVisible(bool visible) { m_gizmoVisible = visible; }
    [[nodiscard]] bool isGizmoVisible() const { return m_gizmoVisible; }

    using PointerCallback = std::function<void(Viewport&, double x, double y)>;

    void setOnPointerDown(PointerCallback callback) { m_onPointerDown = std::move(callback); }
    void setOnPointerMove(PointerCallback callback) { m_onPointerMove = std::move(callback); }
    void setOnPointerUp(PointerCallback callback)   { m_onPointerUp   = std::move(callback); }

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
    ViewportID m_viewportID;
    bool m_gridVisible  = true;
    bool m_axesVisible  = true;
    bool m_gizmoVisible = true;

    PointerCallback m_onPointerDown;
    PointerCallback m_onPointerMove;
    PointerCallback m_onPointerUp;
};

}