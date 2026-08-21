// MirUI/Designer/Application/DesignerState.hpp
// 🎯 Состояние редактора MirUI Designer — режимы, настройки сетки, зум.
//
// DesignerState хранит всё, что описывает текущее состояние самого редактора,
// а не редактируемого документа. Документ — это UIDocument (дерево виджетов, тема).
// А DesignerState — это ответ на вопросы:
//   • В каком режиме мы сейчас? (Select, Move, Resize, AddWidget, Preview)
//   • Показывать ли сетку?
//   • Прилипает ли курсор к сетке?
//   • Какой масштаб (zoom) у холста?
//   • Включён ли режим предпросмотра?
//
// Этот класс используется DesignerApplication для хранения своих настроек,
// а также Canvas, Toolbox, Inspector могут читать его, чтобы подстраивать
// своё поведение (например, в режиме Preview холст не показывает ручки).
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

namespace MirUI {

// ── Режимы работы редактора ──────────────────────────────────
// Это перечисление определяет, что сейчас делает пользователь:
//   Select   — просто выделяет виджеты (кликает или тянет рамку)
//   Move     — перетаскивает выделенные виджеты
//   Resize   — тянет за угол или край, меняет размер
//   AddWidget — перетаскивает новый виджет из тулбокса на холст
//   Preview  — предпросмотр, редактирование отключено
enum class DesignerMode {
    Select,
    Move,
    Resize,
    AddWidget,
    Preview
};

class DesignerState {
public:
    // ── Конструктор ──────────────────────────────────────────
    DesignerState()
        : m_mode(DesignerMode::Select)
        , m_preview(false)
        , m_gridVisible(true)
        , m_snapEnabled(true)
        , m_zoom(1.0)
    {}

    // ── Режим работы ─────────────────────────────────────────
    [[nodiscard]] DesignerMode mode() const { return m_mode; }
    void setMode(DesignerMode mode) { m_mode = mode; }

    // ── Режим предпросмотра ──────────────────────────────────
    // В режиме Preview скрываются ручки, сетка, направляющие,
    // и пользователь видит интерфейс «вживую».
    [[nodiscard]] bool isPreview() const { return m_preview; }
    void setPreview(bool preview) { m_preview = preview; }

    // ── Сетка ─────────────────────────────────────────────────
    // Показывать ли координатную сетку на холсте.
    [[nodiscard]] bool gridVisible() const { return m_gridVisible; }
    void setGridVisible(bool visible) { m_gridVisible = visible; }

    // ── Привязка к сетке (snap) ───────────────────────────────
    // Если включена, координаты при перетаскивании «прилипают»
    // к ближайшим линиям сетки (целочисленные позиции).
    [[nodiscard]] bool snapEnabled() const { return m_snapEnabled; }
    void setSnapEnabled(bool enabled) { m_snapEnabled = enabled; }

    // ── Масштаб (zoom) ───────────────────────────────────────
    // Увеличение холста: 1.0 — 100%, 2.0 — 200% и т.д.
    // Минимальный зум ограничен 0.1, максимальный — 10.0.
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

} // namespace MirUI