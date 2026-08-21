
#pragma once

namespace MirUI {

enum class DesignerMode {
    Select,
    Move,
    Resize,
    AddWidget,
    Preview
};

class DesignerState {
public:

    DesignerState()
        : m_mode(DesignerMode::Select)
        , m_preview(false)
        , m_gridVisible(true)
        , m_snapEnabled(true)
        , m_zoom(1.0)
    {}

    [[nodiscard]] DesignerMode mode() const { return m_mode; }
    void setMode(DesignerMode mode) { m_mode = mode; }

    [[nodiscard]] bool isPreview() const { return m_preview; }
    void setPreview(bool preview) { m_preview = preview; }

    [[nodiscard]] bool gridVisible() const { return m_gridVisible; }
    void setGridVisible(bool visible) { m_gridVisible = visible; }

    [[nodiscard]] bool snapEnabled() const { return m_snapEnabled; }
    void setSnapEnabled(bool enabled) { m_snapEnabled = enabled; }

    [[nodiscard]] double zoom() const { return m_zoom; }
    void setZoom(double zoom) {
        if (zoom < 0.1) zoom = 0.1;
        if (zoom > 10.0) zoom = 10.0;
        m_zoom = zoom;
    }

private:
    DesignerMode m_mode;
    bool m_preview;
    bool m_gridVisible;
    bool m_snapEnabled;
    double m_zoom;
};

}